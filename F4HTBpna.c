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
#include <sys/select.h>

#include "waterfallcolor.c"


bool started = 0;




//##################################Rotary encoder variable
struct btr {
    int pin[3];
	bool pinstate[3];
    bool isready[3];
    unsigned int ttime[3];
	unsigned int ttimelast[3];
    pthread_t t[3];
    int select;
};

btr Arrayofbtr[2];

//##################################Video variable
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
char * framebuffer;
char * fbp = 0;
long int screensize = 0;
int fbfd = 0;
int scale = 1;
int offsetx = 0;
int pixelperdb = 3;
int dbbottom = 90;

int ppx = 0;
int ppy = 0;

bool refreshvalues = 0;

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
char * SOUND_DEVICE = (char * )
                      "default";
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
    printf("FFT Resolution = %f\nFFT out len %d\n", fftw.binWidth, fftw.outlen);
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

float * windowinginit(int N) {

    float * w;

    w = (float * ) calloc(N, sizeof(float));

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
    if (refreshvalues) {
        memset(framebuffer, 0, (screensize / 2 - finfo.line_length) * sizeof( * framebuffer));
        for (unsigned int i = 0; i < vinfo.xres; i++) {
            int y = (254 - * (values + i)) * (ppy -1) / 255;
            for ( int z = y; z < (ppy-1); z++) {
                put_pixel_32bpp(i, z, 255, 255, 255, 0);
            }
        }
    }

}

void pointer_shift(int shift, int offset) {
    shift *= vinfo.xres * vinfo.bits_per_pixel / 8;
    memmove(framebuffer + shift + offset, framebuffer + offset, (screensize - shift - offset) * sizeof( * framebuffer));
}

void setoneFFTline(char * values) {
    pointer_shift(1, (screensize / 2 + finfo.line_length));
    if (refreshvalues) {
        for (unsigned int i = 0; i < vinfo.xres; i++) {
            int y = * (values + i) * 254 / 255;
            put_pixel_32bpp(i, ppy+1, colormap_rainbow[y][0], colormap_rainbow[y][1], colormap_rainbow[y][2], 0);
        }
    } else {
        memcpy(framebuffer + (screensize / 2 + finfo.line_length), framebuffer + (screensize / 2 + finfo.line_length) + finfo.line_length, finfo.line_length);
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

    for (unsigned int x = 0; x < vinfo.xres; x++) put_pixel_32bpp(x, (ppy), 255, 255, 255, 0);
    put_framebuffer_fbp();
}

void print_char_time(char c[], int timesss) {
    if (strlen(c) > 0) strcpy(tmpchar, c);
    if (timesss > 0) secondtextshow = time(NULL) + timesss;
}

void print_char(char c[], unsigned int zoom = 5, int x = -1, int y = -1) {

    if (x == -1 || y == -1) {
        x = ppx - ((strlen(c) * 8 * zoom) / 2);
        y = (ppy / 2) - ((8 * zoom) / 2);
    }

    int numascii = 0;

    for (unsigned int f = 0; f < strlen(c); f++) {
        numascii = (int) c[f] - 32;

        for (unsigned int i = 0; i < (8 * zoom); i++) {
            for (unsigned int j = 0; j < (8 * zoom); j++) {
                if ((font[numascii][i / zoom] >> (7 - j / zoom)) & 0x01) {
                    put_pixel_32bpp(x + f * 8 * zoom + j, y + i, 255, 255, 255, 0);

                } else {
                    put_pixel_32bpp(x + f * 8 * zoom + j, y + i, 0, 0, 0, 0);
                }
            }
        }

    }
}

void updatetextshow() {
    if (time(NULL) < secondtextshow) print_char(tmpchar);
}

void printscale() {
    char word[] = " zoom X   ";
    word[8] = (scale % 10) + '0';
    word[7] = scale / 10 + '0';
    print_char_time(word, 2);
}

void plotscalex() {

    int scaleplotoffset = 5000 / fftw.binWidth;
    int numplotoffset = ppx / scaleplotoffset;

    for (unsigned int y = 0; y < 4; y++) {
        memset(framebuffer + y * finfo.line_length, 255, finfo.line_length);
        memset(framebuffer + screensize - (y + 2) * finfo.line_length, 255, finfo.line_length);
    }

    for (int s = -numplotoffset; s <= numplotoffset; s++) {
        unsigned int k = 1;
        if (s == 0) {
            k = 2;
        }
        for (int x = -1; x < 1; x++) {
            for (unsigned int y = 0; y < 10 * k; y++) put_pixel_32bpp(ppx + x + (s * scaleplotoffset), y + 5, 255, 255, 255, 0);
            for (unsigned int y = 10 * k; y > 0; y--) put_pixel_32bpp(ppx + x + (s * scaleplotoffset), vinfo.yres - 1 - y - 5, 255, 255, 255, 0);
        }
    }
}

void plotscaley() {

    int delta = pixelperdb * (dbbottom - ((dbbottom / 10) * 10));
    int dbdyna = (ppy / pixelperdb);

    /*char word[] = "-  0";
    if (dbbottom % 10) word[3] = (dbbottom % 10);
    if (dbbottom / 10) word[2] = (dbbottom / 10) % 10 + '0';
    if (dbbottom / 100) word[1] = dbbottom / 100 + '0';
	*/
	char word[4];
	sprintf(word, " -%d", dbbottom);
    print_char(word, 1, 0, ppy - 8);

    int y = 10 * pixelperdb;
    while (y < ppy) {

        int ywhere = ppy - y + delta;

        int db = ywhere / pixelperdb - (dbdyna - dbbottom);

        memset(framebuffer + (ywhere) * finfo.line_length, 255, finfo.line_length);

        /*char word[] = "-  0";
        word[2] = db / 10 + '0';
        if (db / 100 != 0) word[1] = db / 100 + '0';
        print_char(word, 1, 0, (ywhere) - 4);
		*/
        
		char word[5];
        sprintf(word, " -%d", db);
        //if (db / 100 != 0) word[1] = db / 100 + '0';
        print_char(word, 1, 0, (ywhere) - 4);
		
		/*db/=10;db*=10;
		char word[4];
		sprintf(word, " -%d", 100);
		print_char(word, 1, 0, (ywhere) - 4);*/
		
        y += (10 * pixelperdb);
    }

    for (unsigned int x = 0; x < 4; x++) {
        for (unsigned int y = 0; y < vinfo.yres; y++) put_pixel_32bpp(x, y, 255, 255, 255, 0);
        for (unsigned int y = 0; y < vinfo.yres; y++) put_pixel_32bpp(vinfo.xres - 1 - x, y, 255, 255, 255, 0);
    }

}

//##################################Inputs fucntions
//Boutton and rotary encoder
void verifoffset() {
    if (2 * abs(offsetx) > ((int) vinfo.xres * ((scale - 1)))) {
        if (offsetx > 0) {
            offsetx = ((int) vinfo.xres * ((scale - 1))) / 2;
        } else {
            offsetx = -((int) vinfo.xres * ((scale - 1))) / 2;
        }
    }
}

void checkrotary(int tr, int trtype) {

    if(tr == 0) {
        if(trtype < 2) {
            if(Arrayofbtr[tr].isready[0] == 1 && Arrayofbtr[tr].isready[1] == 1)
            {
                int sens = 0;
                if(Arrayofbtr[tr].ttime[0] > Arrayofbtr[tr].ttime[1]) {
                    printf("Button %d ++\n",tr);
                    sens=1;
                }
                else {
                    printf("Button %d --\n",tr);
                    sens=-1;
                }
                switch(Arrayofbtr[tr].select) {
                case 0 :

                    if((scale+sens) >0 && (scale+sens)<13) {
                        scale+=sens;
                        flagscalechange = true;
                        printscale();
                        verifoffset();
                    }
                    break;
                case 1 :
                    if ((2 * abs(offsetx + (sens*10*scale)) < ((int) vinfo.xres * ((scale - 1))))) {
                        offsetx+=sens*10*scale;
                    }
                    break;
                case 2 :
                    if((pixelperdb+sens)>1 && (pixelperdb+sens)<30)pixelperdb+=sens;
                    break;
                case 3 :
                    if((dbbottom+sens)>50 && (dbbottom+sens)<150)dbbottom+=sens;
                    break;
                case 4 :
                    if((indexlistofcolorfile+sens)>=0 && (indexlistofcolorfile+sens)<=indexlistofcolorfilemax){
						indexlistofcolorfile+=sens;
					    char fName [128] ;
						snprintf(fName, sizeof fName, "/etc/F4HTBpna/%s%s", listofcolorfile[indexlistofcolorfile],".256");
						printf("%s\n",fName);
						read_csv(fName);
						print_char_time(listofcolorfile[indexlistofcolorfile], 2);
					}
                    break;
                default :
                    printf("Rotary Encoder fault\n" );
                }
                Arrayofbtr[tr].isready[0] = 0;
                Arrayofbtr[tr].isready[1] = 0;
            }
        }
        if(trtype == 2 && Arrayofbtr[tr].isready[2] == 1 && Arrayofbtr[tr].pinstate[2] == 1 && Arrayofbtr[tr].ttime[2] > (Arrayofbtr[tr].ttimelast[2] + 100000)) {
            Arrayofbtr[tr].ttimelast[2] = Arrayofbtr[tr].ttime[2];
			Arrayofbtr[tr].select++;
            if(Arrayofbtr[tr].select>4)Arrayofbtr[tr].select=0;
            switch(Arrayofbtr[tr].select) {
            case 0 :
                print_char_time((const char *)"   Zoom   ", 2);
                break;
            case 1 :
                print_char_time((const char *)"Offset Frq", 2);
                break;
            case 2 :
                print_char_time((const char *)" RF Scale ", 2);
                break;
            case 3 :
                print_char_time((const char *)"Min RF Lvl", 2);
                break;
            case 4 :
                print_char_time((const char *)" WF Color ", 2);
                break;
            default :
                printf("Rotary Encoder fault\n" );
            }

        }
    }

}

int pinread(int pin)
{
#define VALUE_MAX 30
    char path[VALUE_MAX];
    char value_str[3];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (-1 == fd) {
	fprintf(stderr, "Failed to open gpio value for reading!\n");
	return(-1);
    }

    if (-1 == read(fd, value_str, 3)) {
	fprintf(stderr, "Failed to read value!\n");
	return(-1);
    }

    close(fd);

    return(atoi(value_str));
}


void *t_lecture(void *arg)
{
    int *val_p = (int *) arg;

    int tr = val_p[0];
    int trtype = val_p[1];
    int i = Arrayofbtr[tr].pin[trtype];


    int fd;
    fd_set fds;
    char buffer[2];

    printf("inside Starting thread %d,%d on pin:%d\n",tr,trtype,i);

    char fName [128] ;

    sprintf (fName, "/sys/class/gpio/gpio%d/value", i) ;

    if ((fd = open(fName, O_RDONLY)) < 0)
    {
        perror("pin");
        return 0;
    }
    while(1)
    {
        FD_ZERO(&fds);
        FD_SET(fd,&fds);

        if(select(fd+1,NULL,NULL,&fds,NULL) <0)
        {
            perror("select");
            break;
        }

        lseek(fd,0,0);
        if(read(fd,&buffer,2)!=2)
        {
            perror("read");
            break;
        }
		
		
        if((unsigned int)clock() > (Arrayofbtr[tr].ttime[trtype] + 5000)){
		Arrayofbtr[tr].ttime[trtype] = (unsigned int)clock();
        Arrayofbtr[tr].isready[trtype] = 1;
		Arrayofbtr[tr].pinstate[trtype] = pinread(i);
        Arrayofbtr[tr].ttime[trtype] = (unsigned int)clock();
        if(started)checkrotary(tr,trtype);
        printf("pin %d %d ",i,pinread(Arrayofbtr[tr].pin[trtype]));
        fprintf(stdout, "%d\n",Arrayofbtr[tr].ttime[trtype]);
        }








    }

    return NULL;
}

void setpin(int pin)
{
    FILE *fd ;
    if ((fd = fopen ("/sys/class/gpio/export", "w")) == NULL)
    {
        fprintf (stderr, "Unable to open GPIO export interface: %d\n", pin) ;
        exit (1) ;
    }

    fprintf (fd, "%d\n", pin) ;
    fclose (fd) ;
}

void setpininput(int pin) {
    FILE *fd ;
    char fName [128] ;
    sprintf (fName, "/sys/class/gpio/gpio%d/direction", pin) ;

    if ((fd = fopen (fName, "w")) == NULL)
    {
        fprintf (stderr, "Unable to open GPIO direction interface for pin %d\n", pin) ;
        exit (1) ;
    }
    fprintf (fd, "in\n") ;
    fclose (fd) ;

}

void setpinactivelow(int pin) {
    FILE *fd ;
    char fName [128] ;
    sprintf (fName, "/sys/class/gpio/gpio%d/active_low", pin) ;

    if ((fd = fopen (fName, "w")) == NULL)
    {
        fprintf (stderr, "Unable to open GPIO direction interface for pin %d\n", pin) ;
        exit (1) ;
    }
    fprintf (fd, "1\n") ;
    fclose (fd) ;

}

void setpinedge(int pin) {
    FILE *fd ;
    char fName [128] ;
    sprintf (fName, "/sys/class/gpio/gpio%d/edge", pin) ;

    if ((fd = fopen (fName, "w")) == NULL)
    {
        fprintf (stderr, "Unable to open GPIO direction interface for pin %d\n", pin) ;
        exit (1) ;
    }
    fprintf (fd, "rising\n") ;
    fclose (fd) ;

}

void prepbtr(int tr) {

    for(int f = 0; f <3; f++) {
        setpin(Arrayofbtr[tr].pin[f]);
        setpininput(Arrayofbtr[tr].pin[f]);
        setpinactivelow(Arrayofbtr[tr].pin[f]);
        setpinedge(Arrayofbtr[tr].pin[f]);
        int trot[2];
        trot[0] = tr;
        trot[1] = f;
        printf("try starting th %d,%d\n",trot[0],trot[1]);
        pthread_create(&Arrayofbtr[tr].t[f],NULL,t_lecture,(void *)trot);
        usleep(100000);
    }

}

void setupbr() {
    Arrayofbtr[0].pin[0]=17;
    Arrayofbtr[0].pin[1]=27;
    Arrayofbtr[0].pin[2]=26;
    Arrayofbtr[1].pin[0]=24;
    Arrayofbtr[1].pin[1]=23;
    Arrayofbtr[1].pin[2]=13;
    prepbtr(0);
    prepbtr(1);
}

//##################################Inputs fucntions
//Mouse



void * mouse_event(void * arg) {

    int fd, bytes, numinfo = 0;
    unsigned char data[3];

    const char * pDevice = "/dev/input/mice";

    // Open Mouse
    fd = open(pDevice, O_RDWR);
    if (fd == -1) {
        printf("ERROR Opening %s\n", pDevice);

    }

    bool wait = false;
    int left, mouse_x = 0, mouse_y = 0, lastx = 0; //, middle, right
    signed char x, y;
    while (1) {
        // Read Mouse

        bytes = read(fd, data, sizeof(data));

        if (bytes > 0) {
            left = data[0] & 0x1;
            //right = data[0] & 0x2;
            //middle = data[0] & 0x4;

            x = (int) data[1];
            y = (int) data[2];

            mouse_x += x;
            mouse_y += y;

            if (left == 0) {
                wait = false;
                usleep(20000);
                numinfo = 0;
            } else {
                if (mouse_x > 350 && wait == false && flagscalechange == false) {
                    if (mouse_y > 50 && scale <= 12) {
                        wait = true;
                        scale++;
                        flagscalechange = true;
                    } else if (mouse_y < -50 && scale > 1) {
                        wait = true;
                        scale--;
                        flagscalechange = true;
                        verifoffset();
                    }
                    printscale();
                } else {
                    if ((2 * abs(offsetx + (lastx - mouse_x)) < ((int) vinfo.xres * ((scale - 1)))) && numinfo > 2 && abs(x) > 10) {
                        offsetx += (lastx - mouse_x);
                    }
                    lastx = mouse_x;
                    numinfo++;
                }
            }

        }
    }

    (void) arg;
    return NULL;

}

void setup (void)
{

	scandirfilecolor();

    FB_init();

    SOUND_SAMPLES_PER_TURN = vinfo.xres * scale;


    printf("Initialisation of audio and FFT\n");
    audioInit();
    fftwInit();

    printf("Initialisation of blackman harris windowing values\n");
    window = windowinginit(sound.bufferSizeFrames);

    BK_init();

    char word[] = "Startup...";
    print_char_time(word, 2);

    ppx = vinfo.xres / 2;
    ppy = vinfo.yres / 2;

    setupbr();

}

//##################################main function
void help() {
    printf("F4HTBpna -r 192000 -d plughw:CARD=PCH,DEV=0\n or DISAPLAY=:0 sudo ./F4HTBpna -r 192000 -d plughw:CARD=GWloop,DEV=0\n");
    printf("options:\n-r samplerate //set the samplerate from sound card\n-d alsa_device //set alsa device for input \n-m //touchscreen sensitive activation\n-c file  //color file name for watterfall");
    exit(0);
}

int main(int argc, char * argv[]) {
    int c;

    while ((c = getopt(argc, argv, "mhd:r:c:")) != -1)
        switch (c) {
        case 'm':
            pthread_t threadmouseevent;
            pthread_create( & threadmouseevent, NULL, mouse_event, NULL);
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



    setup () ;
    char * values = (char * ) malloc(sizeof(char) * (SOUND_SAMPLES_PER_TURN));

    started = 1;

    while (1) {

        while (audioRead() < 0);

        if (sound.bufferReady) {
            refreshvalues = 0;
            double scalemod = (256 * 256 * 12 * scale);
            for (unsigned int i = 0; i < (unsigned int) sound.bufferSizeFrames; i++) {

                short int valq = getFrame(sound.buffer, i, CLEFT) * window[i];
                short int vali = getFrame(sound.buffer, i, CRIGHT) * window[i];

                fftw.in[i][_Q_] = (((double) vali / scalemod));
                fftw.in[i][_I_] = (((double) valq / scalemod));
            }
            fftw_execute(fftw.plan);

            for (unsigned int i = 0; i < 4; i++) { //DC suppression
                fftw.out[i][0] = 0;
                fftw.out[i][1] = 0;
                fftw.out[SOUND_SAMPLES_PER_TURN - i][0] = 0;
                fftw.out[SOUND_SAMPLES_PER_TURN - i][1] = 0;
            }

            unsigned int i;

            for (int p = 0; p < (int) vinfo.xres; p++) {
                if (p < ((int)ppx - offsetx)) {
                    i = ppx - offsetx - p;
                } else {
                    i = fftw.outlen - p + ppx - offsetx;
                }

                double val = (sqrt(fftw.out[i][0] * fftw.out[i][0] + fftw.out[i][1] * fftw.out[i][1]));
                val = 20 * log10((val)); //convert to db

                val = val > 1.0 ? 1.0 : val;

                /* Save current line for current spectrum. */
                *(values + p) = (char)(val * pixelperdb + (dbbottom * pixelperdb + scale)); //270 is the min value to show

            }
            audioDeinit(); //bether for realtime
            audioInit(); //bether for realtime
            sound.bufferReady = 0;
        } else {
            refreshvalues = 1;
        }

        setoneSPECline(values);
        setoneFFTline(values);
        if (!flagscalechange) {
            updatetextshow();
            plotscaley();
            plotscalex();
        }
        put_framebuffer_fbp();

        if (flagscalechange) {
            fftwDeinit();
            audioDeinit();
            flagscalechange = false;
            sound.reprepare = 1;
            SOUND_SAMPLES_PER_TURN = vinfo.xres * scale;
            audioInit();
            window = windowinginit(sound.bufferSizeFrames);
            fftwInit();
        }

        usleep(10000 / scale); //adjuste in function of time of buffer to do verify
    }


    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}