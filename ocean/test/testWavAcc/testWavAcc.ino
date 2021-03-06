#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// accelerometer
#include <Adafruit_MSA301.h>
#include <Adafruit_Sensor.h>

Adafruit_MSA301 msa;

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav1;     //xy=229,424
AudioSynthWaveformSine   sine1;          //xy=237,319
AudioMixer4              mixer1;         //xy=554,369
AudioOutputI2S           i2s1;           //xy=806,348
AudioConnection          patchCord1(playSdWav1, 0, mixer1, 2);
AudioConnection          patchCord2(playSdWav1, 1, mixer1, 3);
AudioConnection          patchCord3(sine1, 0, mixer1, 1);
AudioConnection          patchCord4(sine1, 0, mixer1, 0);
AudioConnection          patchCord5(mixer1, 0, i2s1, 1);
AudioConnection          patchCord6(mixer1, 0, i2s1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=792,266
// GUItool: end automatically generated code


// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

#define SINE_MIXER 0
#define WAV_MIXER 2

float msaX, msaY, prevMsaX, prevMsaY;

void setup() {
  Serial.begin(9600);

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(8);

  // Comment these out if not using the audio adaptor board.
  // This may wait forever if the SDA & SCL pins lack
  // pullup resistors
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);

  // initialize sine
  sine1.frequency(300);
  sine1.amplitude(0.5);

  // get the sd card
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }

  // accelerometer
  if (! msa.begin()) {
    Serial.println("Failed to find MSA301 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MSA301 Found!");

}

void playFile(const char *filename)
{
  Serial.print("Playing file: ");
  Serial.println(filename);

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playSdWav1.play(filename);

  // A brief delay for the library read WAV info
  delay(25);

  // Simply wait for the file to finish playing.
  while (playSdWav1.isPlaying()) {
    // uncomment these lines if you audio shield
    // has the optional volume pot soldered
    //float vol = analogRead(15);
    //vol = vol / 1024;
    // sgtl5000_1.volume(vol);

    // read the accelerometer
    msa.read();

    msaX = filter(msa.x, prevMsaX, 0.1);
    msaY = filter(msa.y, prevMsaY, 0.1);
    prevMsaX = msaX;
    prevMsaY = msaY;
    float msaMin = abs(min(msaX, msaY));
    // influence the mixer

    msaX += msaMin;
    msaY += msaMin;

    Serial.print(msaX);
    Serial.print(" ");
    Serial.print(msaY);
    Serial.println();
    float msaSum = msaX + msaY;
    setMixer(SINE_MIXER, msaX/msaSum);
    setMixer(WAV_MIXER, msaY/msaSum);

  }
}


void loop() {
  playFile("ocean-waves.wav");  // filenames are always uppercase 8.3 format
  delay(500);
}

float filter(float val, float prevVal, float w)
{
  return w * val + (1 - w) * prevVal;
}

void setMixer(int idx, float val)
{
  mixer1.gain(idx, val);
  mixer1.gain(idx+1, val);
}
