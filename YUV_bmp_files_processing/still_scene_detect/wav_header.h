/*
* Copyleft (c) 2007-2010  Fu Xing (Andy)
* Author: Fu Xing
*
* File name: wav_header.h
* Abstract: LPCM WAVE file header (44 bytes) description
*
* Current version: 1.0
* Last Modified: 2009-5-31
*/


struct   wav
{
	unsigned int    ChunkID;  /* The letters "RIFF" in ASCII form; (0x52494646 big-endian), (0x46464952 little-endian). */
	unsigned int    ChunkSize; /* = (36+SubChunk2Size) = the size of the rest of the chunk following this number = (the size of the entire wav file - 8) */
	unsigned int    Format;  /* The letters "WAVE" in ASCII form; (0x57415645 big-endian), (0x45564157 little-endian). */

	
	unsigned int    Subchunk1ID;  /* The letters "fmt " in ASCII form; (0x666d7420 big-endian), (0x20746d66 little-endian). */
	unsigned int    Subchunk1Size;  /* 16 for PCM.  This is the size of the rest of the Subchunk which follows this number. */
	unsigned short  AudioFormat;  /* LPCM = 1, Values other than 1 indicate some form of compression. */
	unsigned short  NumChannels;  /* Mono = 1, Stereo = 2, etc. */
	unsigned int    SampleRate;  /* number of samples per second; 8000, 44100, etc. */
	unsigned int    ByteRate;  /* SampleRate * NumChannels * BitsPerSample/8 */
	unsigned short  BlockAlign;  /* NumChannels * BitsPerSample/8, i.e. The number of bytes for one sample including all channels. */
                                                              /* I wonder what happens when this number isn't an integer, e.g. 12-bit mono. */
	unsigned short  BitsPerSample;  /* bits per sample per channel, 8 = 8-bit quantization, 16 = 16-bit quantization. */

	unsigned int    Subchunk2ID;  /* The letters "data" in ASCII form; (0x64617461 big-endian), (0x61746164 little-endian). */
	unsigned int    Subchunk2Size;  /* NumSamples * NumChannels * BitsPerSample/8, i.e. the size of the rest of the subchunk following this number */
};


