#ifndef _PLEXON_H_INCLUDED
#define _PLEXON_H_INCLUDED


#define MAX_WF_LENGTH           (56)
#define MAX_WF_LENGTH_LONG      (120)




///////////////////////////////////////////////////////////////////////////////
// Plexon Client API Definitions
///////////////////////////////////////////////////////////////////////////////

//
// PL_Event is used in PL_GetTimestampStructures(...)
//
struct PL_Event
{
    char    Type;                       // PL_SingleWFType, PL_ExtEventType or PL_ADDataType
    char    NumberOfBlocksInRecord;     // reserved   
    char    BlockNumberInRecord;        // reserved 
    unsigned char    UpperTS;           // Upper 8 bits of the 40-bit timestamp
    unsigned long    TimeStamp;         // Lower 32 bits of the 40-bit timestamp
    short   Channel;                    // Channel that this came from, or Event number
    short   Unit;                       // Unit classification, or Event strobe value
    char    DataType;                   // reserved
    char    NumberOfBlocksPerWaveform;  // reserved
    char    BlockNumberForWaveform;     // reserved
    char    NumberOfDataWords;          // number of shorts (2-byte integers) that follow this header 
}; // 16 bytes


//
// The same as PL_Event above, but with Waveform added
//
struct PL_Wave 
{
    char    Type;                       // PL_SingleWFType, PL_ExtEventType or PL_ADDataType
    char    NumberOfBlocksInRecord;     // reserved   
    char    BlockNumberInRecord;        // reserved 
    unsigned char    UpperTS;           // Upper 8 bits of the 40-bit timestamp
    unsigned long    TimeStamp;         // Lower 32 bits of the 40-bit timestamp
    short   Channel;                    // Channel that this came from, or Event number
    short   Unit;                       // Unit classification, or Event strobe value
    char    DataType;                   // reserved
    char    NumberOfBlocksPerWaveform;  // reserved
    char    BlockNumberForWaveform;     // reserved
    char    NumberOfDataWords;          // number of shorts (2-byte integers) that follow this header 
    short   WaveForm[MAX_WF_LENGTH];    // The actual waveform data
}; // size should be 128

//
// An extended version of PL_Wave for longer waveforms
//
struct PL_WaveLong 
{
    char    Type;                       // PL_SingleWFType, PL_ExtEventType or PL_ADDataType
    char    NumberOfBlocksInRecord;     // reserved   
    char    BlockNumberInRecord;        // reserved 
    unsigned char    UpperTS;           // Upper 8 bits of the 40-bit timestamp
    unsigned long    TimeStamp;         // Lower 32 bits of the 40-bit timestamp
    short   Channel;                    // Channel that this came from, or Event number
    short   Unit;                       // Unit classification, or Event strobe value
    char    DataType;                   // reserved
    char    NumberOfBlocksPerWaveform;  // reserved
    char    BlockNumberForWaveform;     // reserved
    char    NumberOfDataWords;          // number of shorts (2-byte integers) that follow this header 
    short   WaveForm[MAX_WF_LENGTH_LONG];   // The actual long waveform data
}; // size should be 256


///////////////////////////////////////////////////////////////////////////////
// Plexon .plx File Structure Definitions
///////////////////////////////////////////////////////////////////////////////


#define LATEST_PLX_FILE_VERSION 105

// file header (is followed by the channel descriptors)
struct PL_FileHeader 
{
    unsigned int MagicNumber;   // = 0x58454c50;

    int     Version;            // Version of the data format; determines which data items are valid
    char    Comment[128];       // User-supplied comment 
    int     ADFrequency;        // Timestamp frequency in hertz
    int     NumDSPChannels;     // Number of DSP channel headers in the file
    int     NumEventChannels;   // Number of Event channel headers in the file
    int     NumSlowChannels;    // Number of A/D channel headers in the file
    int     NumPointsWave;      // Number of data points in waveform
    int     NumPointsPreThr;    // Number of data points before crossing the threshold

    int     Year;               // Time/date when the data was acquired
    int     Month; 
    int     Day; 
    int     Hour; 
    int     Minute; 
    int     Second; 

    int     FastRead;           // reserved
    int     WaveformFreq;       // waveform sampling rate; ADFrequency above is timestamp freq 
    double  LastTimestamp;      // duration of the experimental session, in ticks
    
    // The following 6 items are only valid if Version >= 103
    char    Trodalness;                 // 1 for single, 2 for stereotrode, 4 for tetrode
    char    DataTrodalness;             // trodalness of the data representation
    char    BitsPerSpikeSample;         // ADC resolution for spike waveforms in bits (usually 12)
    char    BitsPerSlowSample;          // ADC resolution for slow-channel data in bits (usually 12)
    unsigned short SpikeMaxMagnitudeMV; // the zero-to-peak voltage in mV for spike waveform adc values (usually 3000)
    unsigned short SlowMaxMagnitudeMV;  // the zero-to-peak voltage in mV for slow-channel waveform adc values (usually 5000)
    
    // Only valid if Version >= 105
    unsigned short SpikePreAmpGain;       // usually either 1000 or 500

    char    Padding[46];      // so that this part of the header is 256 bytes
    
    
    // Counters for the number of timestamps and waveforms in each channel and unit.
    // Note that these only record the counts for the first 4 units in each channel.
    // channel numbers are 1-based - array entry at [0] is unused
    int     TSCounts[130][5]; // number of timestamps[channel][unit]
    int     WFCounts[130][5]; // number of waveforms[channel][unit]

    // Starting at index 300, this array also records the number of samples for the 
    // continuous channels.  Note that since EVCounts has only 512 entries, continuous 
    // channels above channel 211 do not have sample counts.
    int     EVCounts[512];    // number of timestamps[event_number]
};


struct PL_ChanHeader 
{
    char    Name[32];       // Name given to the DSP channel
    char    SIGName[32];    // Name given to the corresponding SIG channel
    int     Channel;        // DSP channel number, 1-based
    int     WFRate;         // When MAP is doing waveform rate limiting, this is limit w/f per sec divided by 10
    int     SIG;            // SIG channel associated with this DSP channel 1 - based
    int     Ref;            // SIG channel used as a Reference signal, 1- based
    int     Gain;           // actual gain divided by SpikePreAmpGain. For pre version 105, actual gain divided by 1000. 
    int     Filter;         // 0 or 1
    int     Threshold;      // Threshold for spike detection in a/d values
    int     Method;         // Method used for sorting units, 1 - boxes, 2 - templates
    int     NUnits;         // number of sorted units
    short   Template[5][64];// Templates used for template sorting, in a/d values
    int     Fit[5];         // Template fit 
    int     SortWidth;      // how many points to use in template sorting (template only)
    short   Boxes[5][2][4]; // the boxes used in boxes sorting
    int     SortBeg;        // beginning of the sorting window to use in template sorting (width defined by SortWidth)
    char    Comment[128];
    int     Padding[11];
};

struct PL_EventHeader 
{
    char    Name[32];       // name given to this event
    int     Channel;        // event number, 1-based
    char    Comment[128];
    int     Padding[33];
};

struct PL_SlowChannelHeader 
{
    char    Name[32];       // name given to this channel
    int     Channel;        // channel number, 0-based
    int     ADFreq;         // digitization frequency
    int     Gain;           // gain at the adc card
    int     Enabled;        // whether this channel is enabled for taking data, 0 or 1
    int     PreAmpGain;     // gain at the preamp

    // As of Version 104, this indicates the spike channel (PL_ChanHeader.Channel) of
    // a spike channel corresponding to this continuous data channel. 
    // <=0 means no associated spike channel.
    int     SpikeChannel;

    char    Comment[128];
    int     Padding[28];
};

// The header for the data record used in the datafile (*.plx)
// This is followed by NumberOfWaveforms*NumberOfWordsInWaveform
// short integers that represent the waveform(s)

struct PL_DataBlockHeader
{
    short   Type;                       // Data type; 1=spike, 4=Event, 5=continuous
    unsigned short   UpperByteOf5ByteTimestamp; // Upper 8 bits of the 40 bit timestamp
    
    #ifdef __x86_64__ 
        #ifdef __MSVC__
            unsigned long TimeStamp;        // Lower 32 bits of the 40 bit timestamp
        #elif __GNUC__
            unsigned int TimeStamp;         // Lower 32 bits of the 40 bit timestamp
        #endif
    #endif
    short   Channel;                    // Channel number
    short   Unit;                       // Sorted unit number; 0=unsorted
    short   NumberOfWaveforms;          // Number of waveforms in the data to folow, usually 0 or 1
    short   NumberOfWordsInWaveform;    // Number of samples per waveform in the data to follow
}; // 16 bytes


///////////////////////////////////////////////////////////////////////////////
// Plexon continuous data file (.DDT) File Structure Definitions
///////////////////////////////////////////////////////////////////////////////

#define LATEST_DDT_FILE_VERSION 103

struct DigFileHeader 
{
    int     Version;        // Version of the data format; determines which data items are valid
    int     DataOffset;     // Offset into the file where the data starts
    double  Freq;           // Digitization frequency
    int     NChannels;      // Number of recorded channels; for version 100-101, this will always
                            // be the same as the highest channel number recorded; for versions >= 102,
                            // NChannels is the same as the number of enabled channels, i.e. channels
                            // whose entry in the ChannelGain array is not 255.

    int     Year;           // Time/date when the data was acquired
    int     Month;
    int     Day;
    int     Hour;
    int     Minute;
    int     Second;
    
    int     Gain;           // As of Version 102, this is the *preamp* gain, not ADC gain
    char    Comment[128];   // User-supplied comment 
    unsigned char BitsPerSample;    // ADC resolution, usually either 12 or 16. Added for ddt Version 101    
    unsigned char ChannelGain[64];  // Gains for each channel; 255 means channel was disabled (not recorded). 
									// The gain for Channel n is located at ChannelGain[n-1]
									// Added for ddt Version 102 
    unsigned char Unused;           // padding to restore alignment 
    short         MaxMagnitudeMV;   // ADC max input voltage in millivolts: 5000 for NI, 2500 for ADS64
                                    // Added for ddt version 103
    unsigned char Padding[188];
};

#endif
