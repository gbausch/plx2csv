///////////////////////////////////////////////////////////////////////////////
// BEGIN LICENSE
//
// Copyright (c) 2011 by Gerold Bausch (mail@geroldbausch.de)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// END LICENSE
///////////////////////////////////////////////////////////////////////////////

#include "plx2csv.h"

int main (int argc, char *argv[])
{

	// Plexon specific structs
	struct PL_FileHeader 		plxFileHeader;
	struct PL_DataBlockHeader	plxDataBlockHeader;
	struct PL_ChanHeader 		plxSpikeChannelHeader;
	struct PL_EventHeader 		plxEventChannelHeader;
	struct PL_SlowChannelHeader	plxSlowChannelHeader;

	// Plexon file information variables
	char 	plxDateTime[25];			// date/time when data was acquired
	char 	plxBitsPerSample;			// Number of bits per sample

	// Options
	char option_NoHeader     = 0;
	char option_HeaderFile   = 0;
	char option_SpikeChannel = 1;
	char option_EventChannel = 0;
	char option_SlowChannel  = 0;

	// misc variables
	FILE *plexonInFile;					// Plexon input file pointer
	FILE *csvOutFile;					// CSV output file pointer
	__int64	timestamp;					// DataBlockHeader time stamp
	double seconds;						// DataBlockHeader time stamp in seconds
	
	char csvOutFileName[255];			// CSV output file name
	short *pWaveformData = NULL;		// Waveform Data pointer

	int i, cntSpikes = 0, cntEvents = 0, cntSlow = 0;
	

	if (argc < 2) {

		printUsage();

	} else {

		// parse cmd line options
		for (i = 0; i < argc; i++) {
			
			if (!strcmp("-noheader", argv[i])) {
				option_NoHeader = 1;
			}

			if (!strcmp("-headerfile", argv[i])) {
				option_NoHeader   = 1;
				option_HeaderFile = 1;
			}

			if (!strcmp("-spikechannel", argv[i])) {
				option_SpikeChannel = 1;
			}
		}


		// start
		plexonInFile = fopen(argv[1], "r+b");
		
		if (plexonInFile != NULL) {

			// create csv output file from input file name
			sprintf(csvOutFileName, "%s.csv", argv[1]);

			// open csv output file name
			csvOutFile = fopen(csvOutFileName, "w+");
			if (csvOutFile != NULL) {
				
				// read plexon file header
				fread(&plxFileHeader, 1, sizeof(struct PL_FileHeader), plexonInFile);

				// exit if header doesn't have magic number
				if (plxFileHeader.MagicNumber != 0x58454C50) {
					
					fprintf(stderr, "Error: Invalid plexon file.\n");

				} else {

					// get date and timestamp from plexon file
					sprintf(plxDateTime, "%4d-%02d-%02d %02d:%02d:%02d", 
						plxFileHeader.Year, 
						plxFileHeader.Month, 
						plxFileHeader.Day, 
						plxFileHeader.Hour, 
						plxFileHeader.Minute, 
						plxFileHeader.Second);

					// get adc resolution for normalization (usually 12 bits)
					if (plxFileHeader.BitsPerSpikeSample == 0) plxBitsPerSample = 12;
					else
						plxBitsPerSample = plxFileHeader.BitsPerSpikeSample;

					fprintf(stdout, "\nplx2csv version 0.1.1, Copyright (c) 2011 by Gerold Bausch\n");					
					fprintf(stdout, " PLX File Version (%d) from %s\n", plxFileHeader.Version, plxDateTime);

					#ifdef __DEBUG__
					fprintf(stdout, " NumDSPChannels    %d\n", plxFileHeader.NumDSPChannels);
					fprintf(stdout, " NumEventChannels  %d\n", plxFileHeader.NumEventChannels);
					fprintf(stdout, " NumSlowChannels   %d\n\n", plxFileHeader.NumSlowChannels);	

					fprintf(stdout, " ADFrequency       %d\n", plxFileHeader.ADFrequency);
					fprintf(stdout, " NumPointsWave     %d\n", plxFileHeader.NumPointsWave);
					fprintf(stdout, " NumPointsPreThr   %d\n", plxFileHeader.NumPointsPreThr);
					#endif

					if (option_NoHeader != 1) {
						fprintf(csvOutFile, "# PLX File Version (%d) from %s\n\n", plxFileHeader.Version, plxDateTime);
						fprintf(csvOutFile, "# ADFrequency=%d\n", plxFileHeader.ADFrequency);
						fprintf(csvOutFile, "# NumPointsWave=%d\n", plxFileHeader.NumPointsWave);
						fprintf(csvOutFile, "# NumPointsPreThr=%d\n\n", plxFileHeader.NumPointsPreThr);
						fprintf(csvOutFile, "# Time (Seconds); Channel; Unit; Data\n");
					}

					// read DSP channel headers (dummy read)
					for (i = 0; i < plxFileHeader.NumDSPChannels; i++) {			
						fread(&plxSpikeChannelHeader, sizeof(struct PL_ChanHeader), 1, plexonInFile);
					}

					// read Event channel headers (dummy read)
					for (i = 0; i < plxFileHeader.NumEventChannels; i++) {			
						fread(&plxEventChannelHeader, sizeof(struct PL_EventHeader), 1, plexonInFile);
					}

					// read Slow channel headers (dummy read)
					for (i = 0; i < plxFileHeader.NumSlowChannels; i++) {			
						fread(&plxSlowChannelHeader, sizeof(struct PL_SlowChannelHeader), 1, plexonInFile);
					}

					// get data from plexon file
					while ((feof(plexonInFile) == 0) && 
						(fread(&plxDataBlockHeader, sizeof(struct PL_DataBlockHeader), 1, plexonInFile)) == 1) {

						// extract timestamp
						timestamp = (((__int64)(plxDataBlockHeader.UpperByteOf5ByteTimestamp))<<32) + 
						 		(__int64)(plxDataBlockHeader.TimeStamp);
						
						// calculate timestamp in seconds
						seconds = (double) timestamp / (double) plxFileHeader.ADFrequency;

						// extract spike channel data
						if ((plxDataBlockHeader.Type == 1) || (option_SpikeChannel == 1)) {
							
							fprintf(csvOutFile, "%.6f,", seconds);
							fprintf(csvOutFile, "%d,", plxDataBlockHeader.Channel);
							fprintf(csvOutFile, "%d,", plxDataBlockHeader.Unit);

							// if block header contains waveforms
							if (plxDataBlockHeader.NumberOfWaveforms == 1) {
								
								// allocate memory and read waveforms
								pWaveformData = (short*) calloc (plxDataBlockHeader.NumberOfWordsInWaveform, sizeof(short));
								fread(&pWaveformData[0], sizeof(short) * plxDataBlockHeader.NumberOfWordsInWaveform, 1, plexonInFile);

								// write waveforms in csv file
								for (i = 0; i < (plxDataBlockHeader.NumberOfWordsInWaveform - 1); i++) {
									fprintf(csvOutFile, "%d,", pWaveformData[i]);
								}
								fprintf(csvOutFile, "%d\n", pWaveformData[plxDataBlockHeader.NumberOfWordsInWaveform-1]);
							}
							// count number of spike events
							cntSpikes++;

						// extract event channel data if selected
						} else if ((plxDataBlockHeader.Type == 4) || (option_EventChannel == 1)) {
							
							// TODO: save event channel information
							cntEvents++;

						// extract slow channel data if selected
						} else if ((plxDataBlockHeader.Type == 5) || (option_SlowChannel == 1)) {

							// TODO: save slow channel information
							cntSlow++;
						}
						
					}
					// print some info on cmd line
					if (cntSpikes > 0) fprintf(stdout, " Found %d spikes ...\n", cntSpikes);
					if (cntEvents > 0) fprintf(stdout, " Found %d events ...\n", cntEvents);
					if (cntSlow > 0) fprintf(stdout, " Found %d continuous ...\n", cntSlow);

					fprintf(stdout, " CSV data ritten in file '%s' ...\n", csvOutFileName);

					// free allocated memory
					free(pWaveformData);
				}

			} else {
				fprintf(stderr, "Error: Cannot open csv output file.\n");
			}
			
			// close csv output file
			fclose(csvOutFile);

			// print separate header file
			if (option_HeaderFile == 1) {

				sprintf(csvOutFileName, "%s.header.csv", argv[1]);
				csvOutFile = fopen(csvOutFileName, "w+");

				fprintf(csvOutFile, "Timestamp,%s\n", plxDateTime);
				fprintf(csvOutFile, "NumDSPChannels,%d\n", plxFileHeader.NumDSPChannels);
				fprintf(csvOutFile, "NumEventChannels,%d\n", plxFileHeader.NumEventChannels);
				fprintf(csvOutFile, "NumSlowChannels,%d\n", plxFileHeader.NumSlowChannels);	
				fprintf(csvOutFile, "ADFrequency,%d\n", plxFileHeader.ADFrequency);
				fprintf(csvOutFile, "NumPointsWave,%d\n", plxFileHeader.NumPointsWave);
				fprintf(csvOutFile, "NumPointsPreThr,%d\n", plxFileHeader.NumPointsPreThr);
				fprintf(csvOutFile, "Waveforms,%d\n", cntSpikes);

				fclose(csvOutFile);
			}

			fprintf(stdout, " Done!\n\n");
			
		} else {
			fprintf(stderr, "Error: Cannot open input file.\n");
		}

		fclose(plexonInFile);
	}

	return 0;
}

void printUsage(void)
{
    
	printf("\n" \
           "plx2csv version 0.1.1, Copyright (c) 2011 by Gerold Bausch\n"\
           "  built on %s %s with gcc %d.%d.%d\n\n"\
           "Plexon file to CSV file converter.\n"\
           "Usage: plx2csv <inputfile> [channel] [options]\n\n"\
           "Options:\n"\
           "  -spikechannel : extract spike channel only (default)\n"\
           "  -eventchannel : extract event channel only\n"\
           "  -slowchannel  : extract slow channel only\n"\
           "  -noheader     : create csv file without header information\n"\
           "  -headerfile   : create separate file for header information\n", __DATE__, __TIME__, __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
}
