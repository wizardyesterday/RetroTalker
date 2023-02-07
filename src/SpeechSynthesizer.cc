//**********************************************************************
// file name: SpeechSynthesizer.cc
//**********************************************************************
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// This class attempts to emulate a Votrax SC-01 speech
// synthesizer.  To achieve this, when presented with a stream of
// phoneme codes, each phoneme code is mapped to a PCM buffer.  The
// contents of this buffer is written to stdout where the output
// can be piped to your favorite audio player.  The format of the
// audio files are 16-bit signed little endian quantities.  The
// sample rate is 16000S/s.
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#include <stdio.h>

#include "SpeechSynthesizer.h"

// Phoneme code context.
struct pcmEntry
{
   char *fileNamePtr;
   int16_t data[8192];
   uint32_t numberOfSamples;
};

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// Each entry of this array represents the context of each phoneme
// code.  The comments on the far right indicate the Votrax SC-01
// phoneme code.
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
static pcmEntry pcmBuffers[] =
{
   {"rawSamples/EH3.raw",{0},0},  // 00
   {"rawSamples/EH2.raw",{0},0},  // 01
   {"rawSamples/EH1.raw",{0},0},  // 02
   {"rawSamples/PA0.raw",{0},0},  // 03
   {"rawSamples/DT.raw",{0},0},   // 04
   {"rawSamples/A2.raw",{0},0},   // 05
   {"rawSamples/A1.raw",{0},0},   // 06
   {"rawSamples/ZH.raw",{0},0},   // 07
   {"rawSamples/AH2.raw",{0},0},  // 08
   {"rawSamples/I3.raw",{0},0},   // 09
   {"rawSamples/I2.raw",{0},0},   // 0A
   {"rawSamples/I1.raw",{0},0},   // 0B
   {"rawSamples/M.raw",{0},0},    // 0C
   {"rawSamples/N.raw",{0},0},    // 0D
   {"rawSamples/B.raw",{0},0},    // 0E
   {"rawSamples/V.raw",{0},0},    // 0F
   {"rawSamples/CH.raw",{0},0},   // 10
   {"rawSamples/SH.raw",{0},0},   // 11
   {"rawSamples/Z.raw",{0},0},    // 12
   {"rawSamples/AW1.raw",{0},0},  // 13
   {"rawSamples/NG.raw",{0},0},   // 14
   {"rawSamples/AH1.raw",{0},0},  // 15
   {"rawSamples/OO1.raw",{0},0},  // 16
   {"rawSamples/OO.raw",{0},0},   // 17
   {"rawSamples/L.raw",{0},0},    // 18
   {"rawSamples/K.raw",{0},0},    // 19
   {"rawSamples/J.raw",{0},0},    // 1A
   {"rawSamples/H.raw",{0},0},    // 1B
   {"rawSamples/G.raw",{0},0},    // 1C
   {"rawSamples/F.raw",{0},0},    // 1D
   {"rawSamples/D.raw",{0},0},    // 1E
   {"rawSamples/S.raw",{0},0},    // 1F
   {"rawSamples/A.raw",{0},0},    // 20
   {"rawSamples/AY.raw",{0},0},   // 21
   {"rawSamples/Y1.raw",{0},0},   // 22
   {"rawSamples/UH3.raw",{0},0},  // 23
   {"rawSamples/AH.raw",{0},0},   // 24
   {"rawSamples/P.raw",{0},0},    // 25
   {"rawSamples/O.raw",{0},0},    // 26
   {"rawSamples/I.raw",{0},0},    // 27
   {"rawSamples/U.raw",{0},0},    // 28
   {"rawSamples/Y.raw",{0},0},    // 29
   {"rawSamples/T.raw",{0},0},    // 2A
   {"rawSamples/R.raw",{0},0},    // 2B
   {"rawSamples/E.raw",{0},0},    // 2C
   {"rawSamples/W.raw",{0},0},    // 2D
   {"rawSamples/AE.raw",{0},0},   // 2E
   {"rawSamples/AE1.raw",{0},0},  // 2F
   {"rawSamples/AW2.raw",{0},0},  // 30
   {"rawSamples/UH2.raw",{0},0},  // 31
   {"rawSamples/UH1.raw",{0},0},  // 32
   {"rawSamples/UH.raw",{0},0},   // 33
   {"rawSamples/O2.raw",{0},0},   // 34
   {"rawSamples/O1.raw",{0},0},   // 35
   {"rawSamples/IU.raw",{0},0},   // 36
   {"rawSamples/U1.raw",{0},0},   // 37
   {"rawSamples/THV.raw",{0},0},  // 38
   {"rawSamples/TH.raw",{0},0},   // 39
   {"rawSamples/ER.raw",{0},0},   // 3A
   {"rawSamples/EH.raw",{0},0},   // 3B
   {"rawSamples/E1.raw",{0},0},   // 3C
   {"rawSamples/AW.raw",{0},0},   // 3D
   {"rawSamples/PA1.raw",{0},0},  // 3E,
   {"rawSamples/STOP.raw",{0},0}  // 3F
};
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

/************************************************************************

  Name: SpeechSynthesizer

  Purpose: The purpose of this function is to serve as the constructor
  of an SpeechSynthesizer object.  The function loads the contents
  of each phoneme into each entry of the phonemeBuffers[] array so that
  the phoneme PCM samples can be output at a later time. 

  Calling Sequence: SpeechSynthesizer(bool& success)

  Inputs:

    success - A reference to storage for the outcome of initialization.
    A value of true insicates sucessful initialization, and a value of
    false indicates unsuccessful initialization.

  Outputs:

    None.

**************************************************************************/
SpeechSynthesizer::SpeechSynthesizer(bool& success)
{
   int i;
   uint32_t numberOfPhonemes;
   FILE *streamPtr;

   // This makes code clearer.
   numberOfPhonemes = sizeof(pcmBuffers) / sizeof(pcmEntry);

   // We need this for bounds checking in the talk() method.
   maximumPhonemeValue = numberOfPhonemes - 1;

   // Default to success.
   success = true;

   for (i = 0; i < numberOfPhonemes; i++)
   {
      // Open the current phoneme PCM file.
      streamPtr = fopen(pcmBuffers[i].fileNamePtr,"r");

      if (streamPtr != NULL)
      {
         // Read in the PCM samples.
         pcmBuffers[i].numberOfSamples =
            fread(pcmBuffers[i].data,sizeof(int16_t),40000,streamPtr);

         // We're done with this phoneme PCM file.
         fclose(streamPtr);
      } // if
      else
      {
         // Indicate failure.
         success = false;
      } // else
   } // for

   return;
 
} // SpeechSynthesizer

/**************************************************************************

  Name: ~SpeechSynthesizer

  Purpose: The purpose of this function is to serve as the destructor
  of a SpeechSynthesizer object.

  Calling Sequence: ~SpeechSynthesizer()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
SpeechSynthesizer::~SpeechSynthesizer(void)
{

   return;

} // ~SpeechSynthesizer

/************************************************************************

  Name: talk

  Purpose: The purpose of this function is to send PCM data to stdiout
  based upon phoneme codes.  Presently, a phoneme code in the
  phoneme buffer will be used to index into a structure that contains
  a buffer, and a buffer length.

  Calling Sequence: talk(uint8_t*& phonemeBuffer,uint32_t phonemeCount)

  Inputs:

    phonemeBuffer - A reference to storage of a binary representation
    of phonemes.

    phonemeCount - The number of phonemes to decode.

  Outputs:

    None.

**************************************************************************/
void SpeechSynthesizer::talk(uint8_t*& phonemeBuffer,
                             uint32_t phonemeCount)
{
   uint32_t i;
   uint8_t phonemeCode;

   for (i = 0; i < phonemeCount; i++)
   {
      // Retrieve the phoneme code from the buffer.
      phonemeCode = phonemeBuffer[i];

      // Make sure we're in bounds.
      if (phonemeCode <= maximumPhonemeValue)
      {
         // Write the PCM samples to stdout.
         fwrite(pcmBuffers[phonemeCode].data,
                sizeof(int16_t),
                pcmBuffers[phonemeCode].numberOfSamples,
                stdout);
      } // if
   } // for
  
   return;

} // talk
