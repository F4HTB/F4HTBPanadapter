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
#include <time.h>

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

char tmpchar[20];
time_t secondtextshow = time(NULL);

static char font[95][8] = {
	{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00 },	/*   (032) 000 */
	{ 0x08,0x08,0x08,0x08,0x08,0x00,0x08 },	/* ! (033) 001 */
	{ 0x66,0x66,0x00,0x00,0x00,0x00,0x00 },	/* " (034) 002 */
	{ 0x24,0x24,0xFF,0x24,0xFF,0x24,0x24 },	/* # (035) 003 */
	{ 0x08,0x3E,0x28,0x3E,0x0A,0x3E,0x08 },	/* $ (036) 004 */
	{ 0x01,0x62,0x64,0x08,0x13,0x23,0x40 },	/* % (037) 005 */
	{ 0x1C,0x22,0x20,0x1D,0x42,0x42,0x3D },	/* & (038) 006 */
	{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00 },	/*   (039) 007 */
	{ 0x02,0x04,0x08,0x08,0x08,0x04,0x02 },	/* ( (040) 008 */
	{ 0x40,0x20,0x10,0x10,0x10,0x20,0x40 },	/* ) (041) 009 */
	{ 0x00,0x49,0x2A,0x1C,0x2A,0x49,0x00 },	/* * (042) 010 */
	{ 0x00,0x08,0x08,0x3E,0x08,0x08,0x00 },	/* + (043) 011 */
	{ 0x00,0x00,0x00,0x00,0x60,0x20,0x40 },	/* , (044) 012 */
	{ 0x00,0x00,0x00,0x7E,0x00,0x00,0x00 },	/* - (045) 013 */
	{ 0x00,0x00,0x00,0x00,0x00,0x60,0x60 },	/* . (046) 014 */
	{ 0x01,0x02,0x04,0x08,0x10,0x20,0x40 },	/* / (047) 015 */
	{ 0x3C,0x42,0x46,0x4A,0x52,0x62,0x3C },	/* 0 (048) 016 */
	{ 0x06,0x0A,0x12,0x02,0x02,0x02,0x02 },	/* 1 (049) 017 */
	{ 0x7E,0x02,0x02,0x7E,0x40,0x40,0x7E },	/* 2 (050) 018 */
	{ 0x7E,0x02,0x02,0x7E,0x02,0x02,0x7E },	/* 3 (051) 019 */
	{ 0x42,0x42,0x42,0x7E,0x02,0x02,0x02 },	/* 4 (052) 020 */
	{ 0x7E,0x40,0x40,0x7E,0x02,0x02,0x7E },	/* 5 (053) 021 */
	{ 0x7E,0x40,0x40,0x7E,0x42,0x42,0x7E },	/* 6 (054) 022 */
	{ 0x7E,0x02,0x04,0x08,0x10,0x20,0x40 },	/* 7 (055) 023 */
	{ 0x7E,0x42,0x42,0x7E,0x42,0x42,0x7E },	/* 8 (056) 024 */
	{ 0x7E,0x42,0x42,0x7E,0x02,0x02,0x7E },	/* 9 (057) 025 */
	{ 0x00,0x00,0x18,0x00,0x18,0x00,0x00 },	/* : (058) 026 */
	{ 0x00,0x00,0x18,0x00,0x18,0x08,0x10 },	/* ; (059) 027 */
	{ 0x04,0x08,0x10,0x20,0x10,0x08,0x04 },	/* < (060) 028 */
	{ 0x00,0x00,0x7E,0x00,0x7E,0x00,0x00 },	/* = (061) 029 */
	{ 0x20,0x10,0x08,0x04,0x08,0x10,0x20 },	/* > (062) 030 */
	{ 0x3E,0x41,0x02,0x04,0x08,0x00,0x08 },	/* ? (063) 031 */
	{ 0x3E,0xC3,0x99,0xA5,0xA6,0xD8,0x7E },	/* @ (064) 032 */
	{ 0x3C,0x42,0x42,0x7E,0x42,0x42,0x42 },	/* A (065) 033 */
	{ 0x7C,0x42,0x42,0x7C,0x42,0x42,0x7C },	/* B (066) 034 */
	{ 0x7E,0x40,0x40,0x40,0x40,0x40,0x7E },	/* C (067) 035 */
	{ 0x7C,0x42,0x42,0x42,0x42,0x42,0x7C },	/* D (068) 036 */
	{ 0x7E,0x40,0x40,0x7E,0x40,0x40,0x7E },	/* E (069) 037 */
	{ 0x7E,0x40,0x40,0x7E,0x40,0x40,0x40 },	/* F (070) 038 */
	{ 0x7E,0x40,0x40,0x5E,0x42,0x42,0x7E },	/* G (071) 039 */
	{ 0x42,0x42,0x42,0x7E,0x42,0x42,0x42 },	/* H (072) 040 */
	{ 0x10,0x10,0x10,0x10,0x10,0x10,0x10 },	/* I (073) 041 */
	{ 0x02,0x02,0x02,0x02,0x02,0x02,0x3C },	/* J (074) 042 */
	{ 0x42,0x44,0x48,0x70,0x48,0x44,0x42 },	/* K (075) 043 */
	{ 0x40,0x40,0x40,0x40,0x40,0x40,0x7E },	/* L (076) 044 */
	{ 0x42,0x66,0x5A,0x42,0x42,0x42,0x42 },	/* M (077) 045 */
	{ 0x41,0x61,0x51,0x49,0x45,0x43,0x41 },	/* N (078) 046 */
	{ 0x3C,0x42,0x42,0x42,0x42,0x42,0x3C },	/* O (079) 047 */
	{ 0x7F,0x41,0x41,0x7F,0x40,0x40,0x40 },	/* P (080) 048 */
	{ 0x3C,0x42,0x42,0x42,0x42,0x42,0x3D },	/* Q (081) 049 */
	{ 0x7C,0x42,0x42,0x7C,0x44,0x42,0x41 },	/* R (082) 050 */
	{ 0x3E,0x40,0x40,0x3C,0x02,0x02,0x7C },	/* S (083) 051 */
	{ 0x7F,0x08,0x08,0x08,0x08,0x08,0x08 },	/* T (084) 052 */
	{ 0x41,0x41,0x41,0x41,0x41,0x41,0x3E },	/* U (085) 053 */
	{ 0x41,0x41,0x41,0x41,0x22,0x14,0x08 },	/* V (086) 054 */
	{ 0x41,0x49,0x49,0x49,0x49,0x49,0x36 },	/* W (087) 055 */
	{ 0x41,0x22,0x14,0x1C,0x14,0x22,0x41 },	/* X (088) 056 */
	{ 0x41,0x22,0x14,0x08,0x08,0x08,0x08 },	/* Y (089) 057 */
	{ 0x7F,0x02,0x04,0x08,0x10,0x20,0x7F },	/* Z (090) 058 */
	{ 0x0F,0x08,0x08,0x08,0x08,0x08,0x0F },	/* [ (091) 059 */
	{ 0x80,0x40,0x28,0x10,0x08,0x04,0x02 },	/* \ (092) 060 */
	{ 0xF8,0x08,0x08,0x08,0x08,0x08,0xF8 },	/* ] (093) 061 */
	{ 0x00,0x14,0x22,0x00,0x00,0x00,0x00 },	/* ^ (094) 062 */
	{ 0x00,0x00,0x00,0x00,0x00,0x00,0xFF },	/* _ (095) 063 */
	{ 0x80,0x20,0x00,0x00,0x00,0x00,0x00 },	/* ` (096) 064 */
	{ 0x3C,0x42,0x42,0x7E,0x42,0x42,0x42 },	/* a (097) 065 */
	{ 0x7C,0x42,0x42,0x7C,0x42,0x42,0x7C },	/* b (098) 066 */
	{ 0x7E,0x40,0x40,0x40,0x40,0x40,0x7E },	/* c (099) 067 */
	{ 0x7C,0x42,0x42,0x42,0x42,0x42,0x7C },	/* d (100) 068 */
	{ 0x7E,0x40,0x40,0x7E,0x40,0x40,0x7E },	/* e (101) 069 */
	{ 0x7E,0x40,0x40,0x7E,0x40,0x40,0x40 },	/* f (102) 070 */
	{ 0x7E,0x40,0x40,0x5E,0x42,0x42,0x7E },	/* g (103) 071 */
	{ 0x42,0x42,0x42,0x7E,0x42,0x42,0x42 },	/* h (104) 072 */
	{ 0x10,0x10,0x10,0x10,0x10,0x10,0x10 },	/* i (105) 073 */
	{ 0x02,0x02,0x02,0x02,0x02,0x02,0x3C },	/* j (106) 074 */
	{ 0x42,0x44,0x48,0x70,0x48,0x44,0x42 },	/* k (107) 075 */
	{ 0x40,0x40,0x40,0x40,0x40,0x40,0x7E },	/* l (108) 076 */
	{ 0x42,0x66,0x5A,0x42,0x42,0x42,0x42 },	/* m (109) 077 */
	{ 0x41,0x61,0x51,0x49,0x45,0x43,0x41 },	/* n (110) 078 */
	{ 0x3C,0x42,0x42,0x42,0x42,0x42,0x3C },	/* o (111) 079 */
	{ 0x7F,0x41,0x41,0x7F,0x40,0x40,0x40 },	/* p (112) 080 */
	{ 0x3C,0x42,0x42,0x42,0x42,0x42,0x3D },	/* q (113) 081 */
	{ 0x7C,0x42,0x42,0x7C,0x44,0x42,0x41 },	/* r (114) 082 */
	{ 0x3E,0x40,0x40,0x3C,0x02,0x02,0x7C },	/* s (115) 083 */
	{ 0x7F,0x08,0x08,0x08,0x08,0x08,0x08 },	/* t (116) 084 */
	{ 0x41,0x41,0x41,0x41,0x41,0x41,0x3E },	/* u (117) 085 */
	{ 0x41,0x41,0x41,0x41,0x22,0x14,0x08 },	/* v (118) 086 */
	{ 0x41,0x49,0x49,0x49,0x49,0x49,0x36 },	/* w (119) 087 */
	{ 0x41,0x22,0x14,0x1C,0x14,0x22,0x41 },	/* x (120) 088 */
	{ 0x41,0x22,0x14,0x08,0x08,0x08,0x08 },	/* y (121) 089 */
	{ 0x7F,0x02,0x04,0x08,0x10,0x20,0x7F },	/* z (122) 090 */
	{ 0x06,0x08,0x08,0x70,0x08,0x08,0x06 },	/* { (123) 091 */
	{ 0x18,0x18,0x18,0x18,0x18,0x18,0x18 },	/* | (124) 092 */
	{ 0x60,0x10,0x10,0x0E,0x10,0x10,0x60 },	/* } (125) 093 */
	{ 0x00,0x00,0x60,0x99,0x06,0x00,0x70 },	/* ~ (126) 094 */
};



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

void print_char_time(char c[], int timesss)
{
	if(strlen(c) > 0)strcpy(tmpchar, c);
	if(timesss > 0) secondtextshow = time(NULL) + timesss;
}



void print_char(char c[],unsigned int zoom = 5, int x = -1, int y = -1){
	
	if(x == -1 || y == -1)
	{
		x = (vinfo.xres/2) -  ((strlen(c)*8*zoom) /2);
		y = (vinfo.yres/4) -  ((8*zoom) /2);
	}

	int numascii = 0;

	for(unsigned int f=0;f<strlen(c);f++){
		numascii = (int)c[f] - 32;

		for(unsigned int i=0; i<(8*zoom); i++){
			for(unsigned int j = 0;  j < (8*zoom) ; j++){
				if((font[numascii][i/zoom] >> (7-j/zoom)) & 0x01)
				{
					put_pixel_32bpp(x + f * 8 * zoom + j, y + i, 255, 255, 255, 0); 
					
				}
				else{put_pixel_32bpp(x + f * 8 * zoom + j, y + i, 0, 0, 0, 0);}
			}
		}

	}
}

void updatetextshow(){
	if(time(NULL) < secondtextshow) print_char(tmpchar);
}

void printscale()
{
	char word[] = " zoom X   "; 
	word[8] = (scale % 10) +'0';
	word[7] = scale/10 +'0';
    print_char_time(word,2);
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
    int left, mouse_x = 0, mouse_y = 0, lastx = 0; //, middle, right
    signed char x, y;
    while(1)
    {
        // Read Mouse
		
        bytes = read(fd, data, sizeof(data));

        if(bytes > 0)
        {
            left = data[0] & 0x1;
            //right = data[0] & 0x2;
            //middle = data[0] & 0x4;

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
					printscale();
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

	  char word[] = "Startup..."; 
	  print_char_time(word,2);

  if (test) {
    while (1) {

      int z = vinfo.xres;
      char * values = (char * ) malloc(sizeof(char) * (z));
      for (int x = 0; x < z; x++) * (values + x) = (char)(rand() % 255);
      setoneSPECline(values);
      setoneFFTline(values);
	  updatetextshow();
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
			
          if(p > ((int)vinfo.xres / 2) -4 && p < ((int)vinfo.xres / 2) +4)val = 3; //DC suppression

          /* Save current line for current spectrum. */
          *(values + p) = (char)(val*3+270); //270 is the min value to show

        }

        setoneSPECline(values);
        setoneFFTline(values);
		if(!flagscalechange)updatetextshow();
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