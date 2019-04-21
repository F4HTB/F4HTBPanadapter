#include "config.h"



#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <alsa/asoundlib.h>
#include <fftw3.h>
#include <math.h>
#include <string.h>
#include <pthread.h>

#include "waterfallcolor.c"

//##################################Video variable
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
char * framebuffer;
char * fbp = 0;
long int screensize = 0;
int fbfd = 0;
int scale = 1;
int offsetx = 0;

//##################################audio and fft variable
int SOUND_RATE = 48000;
char * SOUND_DEVICE = (char * )"default";
int SOUND_SAMPLES_PER_TURN = 2048;
bool flagscalechange = false;
float * window;

/* Global sound info. */
struct soundInfo {
  snd_pcm_t * handle;

  char * buffer, * bufferLast;
  snd_pcm_uframes_t bufferSizeFrames;
  snd_pcm_uframes_t bufferFill;
  int bufferReady;

  int reprepare;
}
sound;

/* Global fftw info. */
struct fftwInfo {
  fftw_complex * in ;
  fftw_complex * out;
  fftw_plan plan;
  int outlen;
  double binWidth;
  double * currentLine;
}
fftw;

//##################################Audio and FFT Function
/* Return the environment variable "name" or "def" if it's unset. */
char * getenvDefault(char * name, char * def) {
  char * val = getenv(name);
  if (val == NULL)
    return def;
  else
    return val;
}

/* Open and init the default recording device. */
void audioInit(void) {
  int rc;
  int size;
  snd_pcm_hw_params_t * params;
  unsigned int val;
  int dir = 0;

  /* Open PCM device for recording (capture). */
  rc = snd_pcm_open( & sound.handle, getenvDefault(SOUND_DEVICE_ENV, SOUND_DEVICE), SND_PCM_STREAM_CAPTURE, 0);
  if (rc < 0) {
    fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
    exit(EXIT_FAILURE);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca( & params);

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(sound.handle, params);

  /* Set the desired hardware parameters. */

  /* Interleaved mode. */
  snd_pcm_hw_params_set_access(sound.handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format. */
  snd_pcm_hw_params_set_format(sound.handle, params, SND_PCM_FORMAT_S16_LE);

  /* twos channels (stereo IQ). */
  snd_pcm_hw_params_set_channels(sound.handle, params, 2);

  /* 44100 ? bits/second sampling rate (CD quality). */
  val = SOUND_RATE;
  snd_pcm_hw_params_set_rate_near(sound.handle, params, & val, & dir);

  /* Set period size. It's best to set this to the same value as
   * SOUND_SAMPLES_PER_TURN. A lower value would generate more
   * hardware interrupts and thus a lower latency but that's of no use
   * since we have to wait for an amount of SOUND_SAMPLES_PER_TURN
   * samples anyway. */
  snd_pcm_uframes_t frames = SOUND_SAMPLES_PER_TURN;
  snd_pcm_hw_params_set_period_size_near(sound.handle, params, & frames, & dir);

  /* Write the parameters to the driver. */
  rc = snd_pcm_hw_params(sound.handle, params);
  if (rc < 0) {
    fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
    exit(EXIT_FAILURE);
  }

  /* Acquire n frames per turn. */
  sound.bufferSizeFrames = SOUND_SAMPLES_PER_TURN;
  size = sound.bufferSizeFrames * 2 * 2; /* 2 bytes/sample, 2 channel */

  /* Initialize the buffer. */
  sound.buffer = (char * ) malloc(size);
  sound.bufferLast = (char * ) malloc(size);
  sound.bufferFill = 0;
  sound.bufferReady = 0;

  /* Try to switch to non-blocking mode for reading. If that fails,
   * print a warning and continue in blocking mode. */
  rc = snd_pcm_nonblock(sound.handle, 1);
  if (rc < 0) {
    fprintf(stderr, "Could not switch to non-blocking mode: %s\n",
      snd_strerror(rc));
  }

  /* Prepare in audioRead() for the first time. */
  sound.reprepare = 1;
}

/* Read as far as you can (when in non-blocking mode) or until our
 * buffer is filled (when in blocking mode). */
int audioRead(void) {
  if (sound.reprepare) {
    int ret;
    ret = snd_pcm_drop(sound.handle);
    if (ret < 0) {
      fprintf(stderr, "Error while dropping samples: %s\n",
        snd_strerror(ret));
    }

    ret = snd_pcm_prepare(sound.handle);
    if (ret < 0) {
      fprintf(stderr, "Error while preparing to record: %s\n",
        snd_strerror(ret));
    }

    sound.reprepare = 0;
  }

  /* Request
   *   "size - fill" frames
   * starting at
   *   "base + numFramesFilled * 2" bytes.
   * Do "* 2" because we get two bytes per sample.
   *
   * When in blocking mode, this always fills the buffer to its
   * maximum capacity.
   */
  snd_pcm_sframes_t rc;
  rc = snd_pcm_readi(sound.handle, sound.buffer + (sound.bufferFill * 2 * 2),
    sound.bufferSizeFrames - sound.bufferFill);
  if (rc == -EPIPE) {
    /* EPIPE means overrun */
    snd_pcm_recover(sound.handle, rc, 0);
  } else if (rc == -EAGAIN) {
    /* Not ready yet. Come back again later. */
  } else if (rc < 0) {
    fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
  } else {
    sound.bufferFill += rc;
    if (sound.bufferFill == sound.bufferSizeFrames) {
      /* Buffer full. display() can add this to the history. */
      sound.bufferFill = 0;
      sound.bufferReady = 1;
    }
  }

  return rc;
}

/* Shutdown audio device. */
void audioDeinit(void) {
  snd_pcm_drop(sound.handle);
  snd_pcm_close(sound.handle);
  free(sound.buffer);
  free(sound.bufferLast);
}

/* Get i'th sample from buffer and convert to short int. */
short int getFrame(char * buffer, int i, short int channel) {
  return (buffer[(4 * i) + channel] & 0xFF) + ((buffer[(4 * i) + 1 + channel] & 0xFF) << 8);
}



/* Create FFTW-plan, allocate buffers. */
void fftwInit(void) {
  fftw.outlen = sound.bufferSizeFrames;

  fftw.in = (fftw_complex * ) fftw_malloc(sizeof(fftw_complex) * sound.bufferSizeFrames);
  fftw.out = (fftw_complex * ) fftw_malloc(sizeof(fftw_complex) * (sound.bufferSizeFrames + 1));

  fftw.plan = fftw_plan_dft_1d(fftw.outlen, fftw.in, fftw.out, FFTW_FORWARD, FFTW_MEASURE);

  fftw.currentLine = (double * ) malloc(sizeof(double) * fftw.outlen);
  memset(fftw.currentLine, 0, sizeof(double) * fftw.outlen);

  /* How many hertz does one "bin" comprise? */
  fftw.binWidth = (double) SOUND_RATE / (double) fftw.outlen;
  printf("FFT Resolution = %f\nFFT out len %d\n", fftw.binWidth,fftw.outlen);
}

/* Free any FFTW resources. */
void fftwDeinit(void) {
  fftw_destroy_plan(fftw.plan);
  fftw_free(fftw.in);
  fftw_free(fftw.out);
  free(fftw.currentLine);
  fftw_cleanup();
}

//##################################Signals fucntions

float *windowinginit(int N){

  float *w;

  w = (float*) calloc(N, sizeof(float));

  //blackman harris windowing values
  float a0 = 0.35875;
  float a1 = 0.48829;
  float a2 = 0.14128;
  float a3 = 0.01168;


  //create blackman harris window
  for (int i = 0; i < N; i++) {
    w[i] = a0 - (a1 * cos((2.0 * M_PI * i) / ((N) - 1))) + (a2 * cos((4.0 * M_PI * i) / ((N) - 1))) - (a3 * cos((6.0 * M_PI * i) / ((N) - 1)));
    //w[i] = 0.5 * (1 - cos((2*M_PI)*i/(NUM_SAMPLES/2))); //hamming window
  }

	return w;

}



//##################################Video fucntions
void put_pixel_32bpp(int x, int y, int r, int g, int b, int t) {
  long int location = 0;
  location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (y + vinfo.yoffset) * finfo.line_length;
  *(framebuffer + location) = b; // Some blue
  *(framebuffer + location + 1) = g; // A little green
  *(framebuffer + location + 2) = r; // A lot of red
  *(framebuffer + location + 3) = t; // No transparency

}

void setoneSPECline(char * values) {
  memset(framebuffer, 0, (screensize / 2 - finfo.line_length) * sizeof( * framebuffer));
  for (unsigned int i = 0; i < vinfo.xres; i++) {
    int y = (254 - * (values + i)) * (vinfo.yres / 2 - 1) / 255;
    for (unsigned int z = y; z < (vinfo.yres / 2 - 1); z++) {
      put_pixel_32bpp(i, z, 255, 255, 255, 0);
    }
  }

}

void pointer_shift(int shift, int offset) {
  shift *= vinfo.xres * vinfo.bits_per_pixel / 8;
  memmove(framebuffer + shift + offset, framebuffer + offset, (screensize - shift - offset) * sizeof( * framebuffer));
}

void setoneFFTline(char * values) {
  pointer_shift(1, (screensize / 2 + finfo.line_length));
  for (unsigned int i = 0; i < vinfo.xres; i++) {
    int y = * (values + i) * 254 / 255;
    put_pixel_32bpp(i, (vinfo.yres / 2 + 1), colormap_rainbow[y][0], colormap_rainbow[y][1], colormap_rainbow[y][2], 0);
  }
}

void put_framebuffer_fbp() {
  memcpy(fbp, framebuffer, screensize);
}

//clear framebuffer
void cfb() {
  for (unsigned int x = 0; x < vinfo.xres; x++)
    for (unsigned int y = 0; y < vinfo.yres; y++)
      put_pixel_32bpp(x, y, 0, 0, 0, 0);
  printf("The framebuffer device was cleared\n");
}

void FB_init() {
  // Open the file for reading and writing
  fbfd = open("/dev/fb0", O_RDWR);
  if (fbfd == -1) {
    perror("Error: cannot open framebuffer device");
    exit(1);
  }
  printf("The framebuffer device was opened successfully.\n");

  // Get fixed screen information
  if (ioctl(fbfd, FBIOGET_FSCREENINFO, & finfo) == -1) {
    perror("Error reading fixed information");
    exit(2);
  }

  // Get variable screen information
  if (ioctl(fbfd, FBIOGET_VSCREENINFO, & vinfo) == -1) {
    perror("Error reading variable information");
    exit(3);
  }

  printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

  // Figure out the size of the screen in bytes
  screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

  framebuffer = (char * ) calloc((screensize), sizeof(char)); //allocate memory which is same size as framebuffer

  // Map the device to memory
  fbp = (char * ) mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
    fbfd, 0);
  if ((int) fbp == -1) {
    perror("Error: failed to map framebuffer device to memory");
    exit(4);
  }
  printf("The framebuffer device was mapped to memory successfully.\n");
}

void BK_init() {
  for (unsigned int x = 0; x < vinfo.xres; x++) put_pixel_32bpp(x, (vinfo.yres / 2), 255, 255, 255, 0);
  put_framebuffer_fbp();
}

//##################################Inputs fucntions
//Mouse

void *mouse_event(void* arg){

	int fd, bytes, numinfo = 0;
    unsigned char data[3];

    const char *pDevice = "/dev/input/mice";

    // Open Mouse
    fd = open(pDevice, O_RDWR);
    if(fd == -1)
    {
        printf("ERROR Opening %s\n", pDevice);
        
    }
	
	bool wait = false;
    int left, middle, right, mouse_x = 0, mouse_y = 0, lastx = 0;
    signed char x, y;
    while(1)
    {
        // Read Mouse
		
        bytes = read(fd, data, sizeof(data));

        if(bytes > 0)
        {
            left = data[0] & 0x1;
            right = data[0] & 0x2;
            middle = data[0] & 0x4;

            x = (int)data[1];
            y = (int)data[2];

			mouse_x += x;
			mouse_y += y;

			if(left == 0){wait=false;usleep(20000);numinfo=0;}
			else{
				if(mouse_x > 350 && wait==false && flagscalechange==false)
				{
					if(mouse_y > 50 && scale <= 12){wait=true;scale++;flagscalechange=true;}
					else if(mouse_y < -50 && scale > 1){
						wait=true;scale--;flagscalechange=true;
						if( 2*abs(offsetx) >  ((int)vinfo.xres*((scale-1))) ){
							if(offsetx>0){offsetx=((int)vinfo.xres*((scale-1)))/2;}
							else{offsetx=-((int)vinfo.xres*((scale-1)))/2;}
						}	
					}
				}
				else{
					if( ( 2*abs(offsetx+(lastx-mouse_x)) <  ((int)vinfo.xres*((scale-1))) ) && numinfo > 2 && abs(x) > 10 ){offsetx+=(lastx-mouse_x);
					}
					lastx=mouse_x;
					numinfo++;
				}
			}
            
        }
    }

	(void) arg;
    return NULL;

}

//##################################main function
void help()
{
	printf("F4HTBpna -r 192000 -d plughw:CARD=PCH,DEV=0\n or DISAPLAY=:0 sudo ./F4HTBpna -r 192000 -d plughw:CARD=GWloop,DEV=0\n");
	printf("options:\n-r samplerate //set the samplerate from sound card\n-d alsa_device //set alsa device for input \n-t //for testing mode \n-m //touchscreen sensitive activation\n-c file  //color file name for watterfall");
	exit(0);
}

int main(int argc, char * argv[]) {
	int c;
	bool test = false;

 

  while ((c = getopt(argc, argv, "mthd:r:c:")) != -1)
    switch (c) {
    case 'm':
      pthread_t threadmouseevent;
	  pthread_create(&threadmouseevent, NULL, mouse_event, NULL);
      break;
      return 1;
    case 't':
      test = true;
      break;
      return 1;
    case 'd':
      SOUND_DEVICE = optarg;
      break;
      return 1;
	case 'c':
      read_csv(optarg);
      break;
      return 1;
    case 'r':
      SOUND_RATE = atoi(optarg);
      break;
      return 1;
    case 'h':
      help();
      exit(0);
      break;
    default:
      help();
      abort();
    }

	 
  
  FB_init();

  SOUND_SAMPLES_PER_TURN =  vinfo.xres * scale; 

  char * values = (char * ) malloc(sizeof(char) * (SOUND_SAMPLES_PER_TURN));

  printf("Initialisation of audio and FFT\n");
  audioInit();
  fftwInit();

  printf("Initialisation of blackman harris windowing values\n");
  window = windowinginit(sound.bufferSizeFrames);

  BK_init();

  if (test) {
    while (1) {

      int z = vinfo.xres;
      char * values = (char * ) malloc(sizeof(char) * (z));
      for (int x = 0; x < z; x++) * (values + x) = (char)(rand() % 255);
      setoneSPECline(values);
      setoneFFTline(values);
      put_framebuffer_fbp();
      usleep(20000);
    }
  } else {
    while (1) {

      while (audioRead() < 0);

      if (sound.bufferReady) {

        for (unsigned int i = 0; i < (unsigned int) sound.bufferSizeFrames; i++) {
			
          short int valq = getFrame(sound.buffer, i, CLEFT) * window[i];
          short int vali = getFrame(sound.buffer, i, CRIGHT)* window[i];

          fftw.in[i][_Q_] = (( (double) vali / (256 * 256 * 12 * scale)));
          fftw.in[i][_I_] = (( (double) valq / (256 * 256 * 12 * scale)));
        }
        fftw_execute(fftw.plan);

        unsigned int i;
        for (int p = 0; p < (int)vinfo.xres; p++) {
          if (p < ((int)(vinfo.xres / 2) - offsetx)) {
            i = (vinfo.xres / 2) - offsetx - p;
          } else {
			i = fftw.outlen - p + (vinfo.xres / 2) - offsetx;
          }
		
          double val = (sqrt(fftw.out[i][0] * fftw.out[i][0] + fftw.out[i][1] * fftw.out[i][1]));
		  val =20*log10((val)); //convert to db

          val = val > 1.0 ? 1.0 : val;
			
          if(p > (vinfo.xres / 2) -4 && p < (vinfo.xres / 2) +4)val = 3; //DC suppression

          /* Save current line for current spectrum. */
          *(values + p) = (char)(val*3+270); //270 is the min value to show

        }

        setoneSPECline(values);
        setoneFFTline(values);
        put_framebuffer_fbp();
		
		if(flagscalechange){
			fftwDeinit();
			audioDeinit();
			flagscalechange=false;
			sound.reprepare = 1;
			SOUND_SAMPLES_PER_TURN = vinfo.xres * scale;
			audioInit();
			window = windowinginit(sound.bufferSizeFrames);
			fftwInit();
		}
		
		audioDeinit();//bether for realtime
		audioInit();//bether for realtime
		

        sound.bufferReady = 0;
		
      }
      usleep(5000 / scale); //adjuste in function of time of buffer to do verify
    }
  }

  munmap(fbp, screensize);
  close(fbfd);
  return 0;
}