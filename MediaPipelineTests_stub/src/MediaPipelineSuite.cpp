/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#include <stdio.h>
#include <unistd.h>
#include <iterator>
#include <string>
#include <vector>
#include <cmath>
#include <sstream>
#include <chrono>
#include <bits/stdc++.h>
#include <sys/wait.h>
extern "C"
{
#include <gst/check/gstcheck.h>
#include <gst/gst.h>
}

using namespace std;


#define PAUSE_TIMEOUT                   5
#define EOS_TIMEOUT 			120
#define DEFAULT_TEST_SUITE_TIMEOUT	360
#define VIDEO_STATUS 			"/CheckVideoStatus.sh"
#define AUDIO_STATUS 			"/CheckAudioStatus.sh"
#define FRAME_DATA                      "/CheckVideoStatus.sh getFrameData "
#define PLAYBIN_ELEMENT 		"playbin"
#define WESTEROS_SINK 			"westerossink"
#define MIN_FRAMES_DROP                 5
#define TOTAL_RESOLUTIONS_COUNT         7 //All resolutions include 144p, 240p, 360p, 480p, 720p, 1080p, 2160p
#define BUFFER_SIZE_LONG		1024
#define BUFFER_SIZE_SHORT		264
#define NORMAL_PLAYBACK_RATE		1.0
#define FCS_MICROSECONDS		1000000
#define Sleep(RunSeconds)               start = std::chrono::steady_clock::now(); \
                                        Runforseconds = RunSeconds; \
                                        while(1) { \
                                        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(Runforseconds)) \
                                             break; \
                                        }
#define WaitForOperation                Sleep(5)
#define AUDIO_DATA                      "/CheckAudioStatus.sh getPTS "
#define AUDIO_PTS_ERROR_FILE            "/audio_pts_error"
#define RESOLUTION_OFFSET               5
#define BUFF_LENGTH 			512

char m_play_url[BUFFER_SIZE_LONG] = {'\0'};
char tcname[BUFFER_SIZE_SHORT] = {'\0'};
char TDK_PATH[BUFFER_SIZE_SHORT] = {'\0'};
char channel_url[BUFFER_SIZE_LONG] = {'\0'};
char audio_change_test[BUFFER_SIZE_SHORT] = {'\0'};
vector<string> operationsList;

/*
 * Default values for avstatus check flag and play_timeout if not received as input arguments
 */

bool checkAVStatus = false;
bool checkPTS = true;
bool bufferUnderflowTest = false;
bool checkSignalTest = true;
int play_timeout = 10;
int videoEnd = 0;
int videoStart =  0;
int audioEnd = 0;
int audioStart = 0;
int fps = 0;
int totalFrames = 0;
int ResolutionCount = 0;
int height,width;
int Runforseconds;
auto start = std::chrono::steady_clock::now();
auto stop_latency = std::chrono::high_resolution_clock::now();
bool latency_check_test = false;
bool firstFrameReceived = false;
bool videoUnderflowReceived = false;
bool checkTotalFrames = false;
int SecondChannelTimeout =0;
bool ChannelChangeTest = false;
GstClockTime timestamp, latency, time_elapsed;
bool writeToFile= false;
FILE *filePointer;
bool justPrintPTS = false;
bool seekOperation = false;
gint seekSeconds = 0;
bool play_without_video = false;
bool play_without_audio = false;
bool ResolutionTest = false;
bool ResolutionSwitchTest = false;
string resolution;
bool resolution_test_up = true;
bool with_pause = false;
bool useProcForFPS = false;
bool UHD_Not_Supported = false;
bool use_rialto = false;
bool audio_underflow_received = false;
bool audio_underflow_received_global = false;
bool audio_started  = false;
bool audio_pts_error = false;
bool AudioPTSCheckAvailable = false;
bool Flush_Pipeline = true;
bool use_westerossink_fps = true;
bool use_audioSink = true;
gint flags;
bool ignorePlayJump = false;
bool buffering_flag = true;
bool checkAudioSamplingrate = false;
int sampling_rate = 0;
guint64 connection_speed;
int bit_depth;
int Bit_depth_got;
bool brcm = false;
bool rtk = false;


/*
 * Playbin flags
 */
/*
 * GstPlayFlags flags from playbin2. It is the policy of GStreamer to
 * not publicly expose element-specific enums. That's why this
 * GstPlayFlags enum has been copied here.
 */
typedef enum {
  GST_PLAY_FLAG_VIDEO         = (1 << 0),
  GST_PLAY_FLAG_AUDIO         = (1 << 1),
  GST_PLAY_FLAG_TEXT          = (1 << 2),
  GST_PLAY_FLAG_VIS           = (1 << 3),
  GST_PLAY_FLAG_SOFT_VOLUME   = (1 << 4),
  GST_PLAY_FLAG_NATIVE_AUDIO  = (1 << 5),
  GST_PLAY_FLAG_NATIVE_VIDEO  = (1 << 6),
  GST_PLAY_FLAG_DOWNLOAD      = (1 << 7),
  GST_PLAY_FLAG_BUFFERING     = (1 << 8),
  GST_PLAY_FLAG_DEINTERLACE   = (1 << 9),
  GST_PLAY_FLAG_SOFT_COLORBALANCE = (1 << 10),
  GST_PLAY_FLAG_FORCE_FILTERS = (1 << 11),
  GST_PLAY_FLAG_FORCE_SW_DECODERS = (1 << 12),
} GstPlayFlags;

/*
 * Structure to pass arguments to/from the message handling method
 */
typedef struct CustomData {
    GstElement *playbin;                /* Playbin element handle */
    GstState cur_state;                 /* Variable to store the current state of pipeline */
    gint64 seekPosition;                /* Variable to store the position to be seeked */
    gdouble seekSeconds;                /* Variable to store the position to be seeked in seconds */
    gint64 currentPosition;             /* Variable to store the current position of pipeline */
    gint64 duration;                    /* Variable to store the duration  of the piepline */
    gboolean terminate;                 /* Variable to indicate whether execution should be terminated in case of an error */
    gboolean seeked;                    /* Variable to indicate if seek to requested position is completed */
    gboolean eosDetected;               /* Variable to indicate if EOS is detected */
    gboolean stateChanged;              /* Variable to indicate if stateChange is occured */
    gboolean streamStart;               /* Variable to indicate start of new stream */
    gboolean setRateOperation;          /* Variable which indicates setRate operation is carried out */
    gdouble setRate;                    /* Variable to indicate the playback rate to be set */
    gdouble currentRate;                /* Variable to store the current playback rate of the pipeline */
    gint n_text;                        /* Number of embedded text streams */
    gint n_audio;                       /* Number of embedded audio streams */
    gint n_video;                       /* Number of embedded video streams */
    gint current_video;                 /* Currently playing video streams */
    gint current_text;                  /* Currently playing text stream */
    gint current_audio;                 /* Currently playing audio stream */
} MessageHandlerData;

/*
 * Methods
 */
static void handleMessage (MessageHandlerData *data, GstMessage *message);

void assert_failure(GstElement* playbin, bool success, const char *str= "Failure occured")
{
   if(success)
      return;
   if(playbin)
   {
      gst_element_set_state (playbin, GST_STATE_NULL);
   }
   gst_object_unref (playbin);
   fail_unless(false,str);
}
 
/*******************************************************************************************************************************************
Purpose:                To continue the state of the pipeline and check whether operation is being carried throughout the specified interval
Parameters:
playbin                   - The pipeline which is to be monitored
RunSeconds:               - The interval for which pipeline should be monitored
********************************************************************************************************************************************/
static void PlaySeconds(GstElement* playbin,int RunSeconds)
{
   gint64 startPosition;
   gint64 currentPosition;
   gfloat difference;
   GstMessage *message;
   GstBus *bus;
   MessageHandlerData data;
   gint64 pts;
   gint64 old_pts;
   gint pts_buffer=5;
   gint RanForTime=0;
   GstElement *videoSink;
   GstElement *audioSink;
   GstState cur_state;
   gfloat _play_jump = 0;
   int play_jump =0; 
   int play_jump_previous = 99;
   gint jump_buffer = 3;
   gint frame_buffer = 3;
   gint audio_frame_buffer = 3;
   gint jump_buffer_small_value = 2;
   GstStateChangeReturn state_change;
   guint64 previous_rendered_frames;
   //gint queued_frames;
   guint64 dropped_frames;
   guint64 rendered_frames;
   guint64 dropped_frames_audio;
   guint64 rendered_frames_audio;
   guint64 previous_rendered_frames_audio;
   guint64 audio_sampling_rate;
   int audio_sampling_rate_buffer = 3;
   int audio_sampling_rate_diff = 0;
   int audio_sampling_diff = 0;
   int rate_diff_threshold = -3;
   float drop_rate;
   int frame_rate;
   float dropped_percentage;
   vector<int> resList;
   int resItr = 0;
   GstStructure *structure;
   GstStructure *audio_structure;
   gfloat _currentPosition = 0;
   gfloat previous_position = 0;
   gfloat _startPosition = 0;
   gint current_position= 0;

   /* Update data variables */
   data.playbin = playbin;
   data.setRateOperation = FALSE;
   data.terminate = FALSE;
   data.eosDetected = FALSE;

   if (ResolutionSwitchTest || ResolutionTest)
	   use_audioSink = false;
   
   if (ResolutionSwitchTest)
   {
        resList.push_back(144);
        resList.push_back(240);
        resList.push_back(360);
        resList.push_back(480);
        resList.push_back(720);
        resList.push_back(1080);
	if (!UHD_Not_Supported)
           resList.push_back(2160);
        if (!resolution_test_up)
           reverse(resList.begin(), resList.end());
   }

   if (seekOperation)
   {
       startPosition = seekSeconds * GST_SECOND;
   }
   else
   {
       gst_element_get_state (playbin, &cur_state, NULL, GST_SECOND);
       assert_failure (playbin,gst_element_query_position (playbin, GST_FORMAT_TIME, &startPosition), "Failed to query the current playback position");
   }
   g_object_get (playbin,"video-sink",&videoSink,NULL);

   g_object_get (playbin,"audio-sink",&audioSink,NULL);

   if (checkPTS)
   {
        g_object_get (videoSink,"video-pts",&pts,NULL);
        old_pts = pts;
   }

   printf("\nRunning for %d seconds, start Position is %lld\n",RunSeconds,startPosition/GST_SECOND);
   previous_position = (startPosition/GST_SECOND);
   if (seekOperation)
   {
       previous_position = seekSeconds;
   }

   if ((cur_state == GST_STATE_PAUSED)  && (state_change != GST_STATE_CHANGE_ASYNC))
   {
        do
        {
            Sleep(1);
            printf("\nCurrent State is PAUSED , waiting for %d\n", RunSeconds);
	    assert_failure (playbin,gst_element_query_position (playbin, GST_FORMAT_TIME, &currentPosition), "Failed to query the current playback position");
	    _currentPosition = currentPosition;
	    play_jump = (_currentPosition/GST_SECOND) - previous_position;
            previous_position = (_currentPosition/GST_SECOND);
            printf("Current Position : %0.2f\n",(_currentPosition/GST_SECOND));
	    if (play_jump != 0)
            {
                printf("Playback is not PAUSED");
                jump_buffer_small_value -=1;
            }

	    if (checkPTS)
            {		    
                g_object_get (videoSink,"video-pts",&pts,NULL);
                printf("\nPTS: %lld \n",pts);
                if (old_pts != pts)
                {
                    pts_buffer -= 1;
                }
		if(!justPrintPTS)
		{
                   assert_failure (playbin,pts_buffer != 0 , "Video is not PAUSED according to video-pts check of westerosSink");
	           assert_failure (playbin,old_pts != 0 , "Video is not rendered according to video-pts check of westerosSink");
		}
	        old_pts = pts;
	    }
	    assert_failure (playbin,jump_buffer_small_value !=0 ,"FAILURE:Playback is not paused");
            RanForTime += 1;
        }while(RanForTime < RunSeconds);
        return;
   }


   bus = gst_element_get_bus (playbin);
   if (use_westerossink_fps)
   {
	g_object_get (videoSink,"stats",&structure,NULL);
        gst_structure_get_uint64(structure, "rendered", &rendered_frames);
   }
   if (use_audioSink)
   {
        g_object_get (audioSink,"stats",&audio_structure,NULL);
        gst_structure_get_uint64(audio_structure, "rendered", &rendered_frames_audio);
	previous_rendered_frames_audio = rendered_frames_audio;
   }

   
   do
   {
	Sleep(1);
	//g_object_get (videoSink,"queued-frames",&queued_frames,NULL);
        //printf("\nQueued Frames = %d",queued_frames);
	if (audio_underflow_received)
            audio_underflow_received_global = true;
        audio_underflow_received = false;
        if (use_westerossink_fps)
        {
             g_object_get (videoSink,"stats",&structure,NULL);

	     previous_rendered_frames = rendered_frames;

             if (structure && (gst_structure_has_field(structure, "dropped") || gst_structure_has_field(structure, "rendered")))
             {
                 gst_structure_get_uint64(structure, "dropped", &dropped_frames);
                 gst_structure_get_uint64(structure, "rendered", &rendered_frames);
		 printf("\n\nVIDEO_FRAMES ");
                 printf(" Dropped: %" G_GUINT64_FORMAT, dropped_frames);
                 printf(" Rendered: %" G_GUINT64_FORMAT, rendered_frames);
             }

	     assert_failure (playbin,rendered_frames != 0 , "Video rendered_frames is coming as 0");  
            
             if ((rendered_frames <= previous_rendered_frames))
	     	frame_buffer -= 1;
	     if (frame_buffer == 0)
		gst_object_unref (bus);
             assert_failure (playbin,frame_buffer != 0 , "Video frames are not rendered properly");    
	     
        }

	
	if (use_audioSink)
        {
             g_object_get (audioSink,"stats",&audio_structure,NULL);

             if (audio_structure && (gst_structure_has_field(audio_structure, "dropped") || gst_structure_has_field(audio_structure, "rendered")))
             {
                 gst_structure_get_uint64(audio_structure, "dropped", &dropped_frames_audio);
                 gst_structure_get_uint64(audio_structure, "rendered", &rendered_frames_audio);
		 printf("\nAUDIO_FRAMES ");
                 printf(" Dropped: %" G_GUINT64_FORMAT, dropped_frames_audio);
                 printf(" Rendered: %" G_GUINT64_FORMAT, rendered_frames_audio);
             }
	     if ((rendered_frames_audio <= previous_rendered_frames_audio))
                audio_frame_buffer -= 1;
	     if (audio_frame_buffer == 0)
		 gst_object_unref (bus);
             assert_failure (playbin,audio_frame_buffer != 0 , "Audio frames are not rendered properly");
	     if (checkAudioSamplingrate)
	     {
		audio_sampling_rate = rendered_frames_audio - previous_rendered_frames_audio;
    		printf("\nAudio sampling rate = %" G_GUINT64_FORMAT, audio_sampling_rate);
    		previous_rendered_frames_audio = rendered_frames_audio;


		audio_sampling_diff = sampling_rate - (int)audio_sampling_rate;
		printf(" Sampling rate = %d",sampling_rate);
		printf(" Audio_sampling_rate_diff = %d",audio_sampling_diff);


		if (audio_sampling_diff > 0)
		{
			if (audio_sampling_diff <= rate_diff_threshold)
				audio_sampling_rate_buffer -= 1;
		}
                if (audio_sampling_rate_buffer == 0)
		    gst_object_unref (bus);
		assert_failure (playbin,audio_sampling_rate_buffer != 0, "Audio sampling rate was not rendered properly");
	     }

        }


        assert_failure (playbin,gst_element_query_position (playbin, GST_FORMAT_TIME, &currentPosition), "Failed to query the current playback position");
	_currentPosition = currentPosition;
	_startPosition = startPosition;
        difference = abs((_currentPosition/GST_SECOND) - (_startPosition/GST_SECOND));
        printf("\nCurrent Position : %0.2f , Playing after operation for: %0.2f",(_currentPosition/GST_SECOND),difference);

	_play_jump = (_currentPosition/GST_SECOND) - previous_position;
	printf("\nPlay jump = %0.2f", _play_jump);
	play_jump = round(_play_jump);

        if (ResolutionSwitchTest)
	{
            int new_height, new_width;
	    g_object_get (videoSink, "video-height", &new_height, NULL);
            g_object_get (videoSink, "video-width", &new_width, NULL);
	    if (abs(height - resList[resItr]) < RESOLUTION_OFFSET)
	    {
		ResolutionCount++;
		resItr++;
            }
            if ( (new_height != height) || (new_width != width))
            {
                height = new_height;
                width = new_width;
                printf("\nVideo height = %d\nVideo width = %d", height, width);
	        printf("\nres-comparison value = %d",resList[resItr]);
            }	
	}
	/*
	 * Ignore if first jump is 0
	 */
        if ((play_jump != NORMAL_PLAYBACK_RATE) && !(((play_jump == 0) && (play_jump_previous == 99))) && !(play_jump == -1))
	{
            jump_buffer -=1;
        }
	/*
	 * For small jumps until 2 , jump_buffer is 2
	 */
        if (((play_jump == 0) || (play_jump == 2) || (play_jump == -1))&& (jump_buffer == 0))
	{
            jump_buffer_small_value -=1;
	    jump_buffer = jump_buffer_small_value;
        }

	/*
	 * if playbin reports jump=0 and then jump=2 , then video has played fine only
	 */
	if (((play_jump == 2) && (play_jump_previous == 0)) && (jump_buffer == 0))
	{
	    jump_buffer = 1;
	}
	if (((play_jump == 0) && (play_jump_previous == 2)) && (jump_buffer == 0))
	{
	    jump_buffer = 1;
	}

	if (play_jump > 3)
	{
		jump_buffer = 0;
	}

        message = gst_bus_pop_filtered (bus, (GstMessageType) ((GstMessageType) GST_MESSAGE_STATE_CHANGED |
                                             (GstMessageType) GST_MESSAGE_ERROR | (GstMessageType) GST_MESSAGE_EOS |
                                             (GstMessageType) GST_MESSAGE_ASYNC_DONE ));

        /*
         * Parse message
         */
        if (NULL != message)
        {
            handleMessage (&data, message);
        }

	if (checkPTS)
	{
	    g_object_get (videoSink,"video-pts",&pts,NULL);
            printf("\nPTS: %lld",pts);
            if ((pts ==0) || (old_pts >= pts))
            {
	        pts_buffer -= 1;
		printf("\nWARNING : Video not playing");
            }
            if (!justPrintPTS)
	    {
		if (pts_buffer == 0)
	            gst_object_unref (bus);	    
                assert_failure (playbin,pts_buffer != 0 , "Video is not playing according to video-pts check of westerosSink");
	    }
	    old_pts = pts;
	}

	if ((jump_buffer == 0) && (ResolutionSwitchTest)) 
             printf("\nVideo height = %d\nVideo width = %d", height, width);


	if (!ignorePlayJump)
	{
	     if (jump_buffer == 0)
		 gst_object_unref (bus);
	     assert_failure (playbin,jump_buffer != 0 , "Playback is not happening at the expected rate");
	}

	previous_position = (_currentPosition/GST_SECOND);
	if(videoUnderflowReceived && bufferUnderflowTest)
        {
            printf("\nVideo Underflow received breaking from PlaySeconds");
            printf("\nExiting from PlaySeconds, currentPosition is %lld\n",currentPosition/GST_SECOND);
            gst_object_unref (bus);
	    Sleep(3);
            return;
        }

	if(audio_underflow_received_global && bufferUnderflowTest)
        {
            printf("\nAudio Underflow received breaking from PlaySeconds");
            printf("\nExiting from PlaySeconds, currentPosition is %lld\n",currentPosition/GST_SECOND);
            gst_object_unref (bus);
	    Sleep(3);
            return;
        }

        if((ResolutionSwitchTest) && (ResolutionCount == TOTAL_RESOLUTIONS_COUNT))
	{
	   printf("\nPipeline played all resolutions\n");
           break;
	}
	play_jump_previous = play_jump;

        if(use_westerossink_fps)
        {
   	   frame_rate = int(rendered_frames - previous_rendered_frames);
	   current_position = round(currentPosition/GST_SECOND);
   	   totalFrames = frame_rate * current_position;
   	   dropped_percentage = (dropped_frames/totalFrames);
   	   if(dropped_percentage != 0.00)
	   	printf("\nDropped Percentage : %f\n", dropped_percentage);
	   if (dropped_percentage >= 1.0)
		gst_object_unref (bus);
   	   assert_failure (playbin,dropped_percentage < 1.0 , "Dropped frames are observed");
        }
	RanForTime++;
   }while((RanForTime < RunSeconds) && !data.terminate && !data.eosDetected);
 
   if (use_audioSink)
   {
   	if (checkAudioSamplingrate)
   	{	   	
   		if (sampling_rate == 96)
                     rate_diff_threshold = 5;
   	}
   }

   printf("\nExiting from PlaySeconds, currentPosition is %0.2f\n",(_currentPosition/GST_SECOND));
   gst_object_unref (bus);
}

/*
 * Methods
 */

/********************************************************************************************************************
	Purpose: Setflag function to set the flags
*********************************************************************************************************************/
void setflags() 
{
        const char* substr = "aamp";
	if (std::strstr(m_play_url, substr))
        {
            printf("\nAAMP is used as plugin\n");
            flags |= 0x03 | 0x00000040;
	    return;
        }
	flags= GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_AUDIO;
	if (buffering_flag)
	{
	    flags |= GST_PLAY_FLAG_BUFFERING;
	}
#ifdef NATIVE_AUDIO
	flags |= GST_PLAY_FLAG_NATIVE_AUDIO;
#endif
#ifndef NO_NATIVE_VIDEO
    	flags |= GST_PLAY_FLAG_NATIVE_VIDEO;
#endif
}

/********************************************************************************************************************
Purpose:               To get the current status of the AV running
Parameters:
Return:               - bool SUCCESS/FAILURE
*********************************************************************************************************************/
bool getstreamingstatus(char* script)
{
    char buffer[BUFFER_SIZE_SHORT]={'\0'};
    char result[BUFFER_SIZE_LONG]={'\0'};
    FILE* pipe = popen(script, "r");
    if (!pipe)
    {
            GST_ERROR("Error in opening pipe \n");
            return false;
    }
    while (!feof(pipe)) 
    {
        if (fgets(buffer, BUFFER_SIZE_SHORT, pipe) != NULL)
        {
            strcat(result, buffer);
        }
    }
    pclose(pipe);
    GST_LOG("Script Output: %s %s\n", script, result);
    if (strstr(result, "SUCCESS") != NULL)
    {
    	return true;
    }
    else
    {
        return false;
    }
}


/******************************************************************************************************************
 * Purpose:                Callback function to capture underflow signal from audiosink
 *****************************************************************************************************************/
static void AudioUndeflowCallback(GstElement *sink, guint size, void *context, gpointer data)
{
   bool *gotVideoUnderflow = (bool*)data;
   if (!audio_underflow_received)
   printf ("\nERROR : Received audio buffer underflow signal\n");
   *gotVideoUnderflow = true;
}

/******************************************************************************************************************
 * Purpose:                Callback function to capture first frame signal from audiosink
 *****************************************************************************************************************/
static void FirstFrameAudioCallback(GstElement *sink, guint size, void *context, gpointer data)
{
   bool *gotFirstFrameSignal = (bool*)data;
   printf ("\nINFO : Received First Audio frame signal\n");
   *gotFirstFrameSignal = true;
}

/******************************************************************************************************************
 * Purpose:                Callback function to capture pts error signal from audiosink
 *****************************************************************************************************************/
static void AudioPTSErrorCallback(GstElement *sink, guint size, void *context, gpointer data)
{
   bool *gotPTSerror = (bool*)data;
   printf("\nERROR : Received Audio PTS error\n");
   *gotPTSerror = true;
}

/******************************************************************************************************************
 * Purpose:                Callback function to setup up element signals with callback functions
 *****************************************************************************************************************/
static void elementSetupCallback (GstElement* playbin, GstElement* element, gpointer user_data)
{
    if (g_str_has_prefix(GST_ELEMENT_NAME(element), "brcmaudiodecoder")) {
        printf("\nGot brcmaudiodecoder\n");
	brcm = true;
        g_object_set(G_OBJECT(element), "enable-svp", true , NULL);
        g_signal_connect( element, "buffer-underflow-callback", G_CALLBACK(AudioUndeflowCallback), &audio_underflow_received);
        g_signal_connect( element, "first-audio-frame-callback", G_CALLBACK(FirstFrameAudioCallback), &audio_started);
        g_signal_connect( element, "pts-error-callback", G_CALLBACK(AudioPTSErrorCallback), &audio_pts_error);
    }
    else if (g_str_has_prefix(GST_ELEMENT_NAME(element), "amlhalasink")) {
        printf("\nGot amlhalasink\n");
        g_signal_connect( element, "underrun-callback", G_CALLBACK(AudioUndeflowCallback), &audio_underflow_received);
    }
    else if (g_str_has_prefix(GST_ELEMENT_NAME(element), "rtkaudiosink")) {
        printf("\nGot rtkaudiosink\n");
	rtk = true;
        g_signal_connect( element, "buffer-underflow-callback", G_CALLBACK(AudioUndeflowCallback), &audio_underflow_received);
    }

}


static void parselatency()
{
    printf("\nLatency = %" GST_TIME_FORMAT"\n",GST_TIME_ARGS(latency));
    latency = GST_TIME_AS_MSECONDS(latency);
    printf("\nLatency = %lld milliseconds\n", latency);
    /*
     * Writing to file
     */
    FILE *filePointer ;
    char latency_file[BUFFER_SIZE_SHORT] = {'\0'};
    strcat (latency_file, TDK_PATH);
    strcat (latency_file, "/latency_log");
    filePointer = fopen(latency_file, "w");
    if (filePointer != NULL)
    {
        fprintf(filePointer,"Latency = %lld milliseconds\n", latency);
    }
    else
    {
	printf("\nLatency writing operation failed\n");
    }
    fclose(filePointer);
}

/* Print all propeties of the stream represented in the tag */
static void printTag (const GstTagList *tags, const gchar *tag, gpointer user_data)
{
    GValue value = { 0, };
    gchar *Data;
    gint depth = GPOINTER_TO_INT (user_data);
    
    gst_tag_list_copy_value (&value, tags, tag);

    if (G_VALUE_HOLDS_STRING (&value))
        Data = g_value_dup_string (&value);
    else
        Data = gst_value_serialize (&value);

    g_print ("%*s%s: %s\n", 2 * depth, " ", gst_tag_get_nick (tag), Data);
    if (writeToFile == true)
        fprintf(filePointer,"%*s%s: %s\n", 2 * depth, " ", gst_tag_get_nick (tag), Data);
    g_free (Data);
    g_value_unset (&value);
}

/*
 * Based on GStreamer sample code, which is:
 * Copyright (C) GStreamer developers
 * Here licensed under the MIT license
 */

static void getStreamProperties (MessageHandlerData *data)
{
    gint i;
    GstTagList *tags;

    /* Read some properties */
    g_object_get (data->playbin, "n-video", &data->n_video, NULL);
    g_object_get (data->playbin, "n-audio", &data->n_audio, NULL);

    g_print ("Stream contains %d audio stream(s)\n",data->n_audio);

    /* Retrieve the stream's video tags
    g_print ("\n");
    for (i = 0; i < data->n_video; i++) 
    {
        tags = NULL;
        g_signal_emit_by_name (data->playbin, "get-video-tags", i, &tags);
        if (tags) 
	{
           g_print ("Tags:\n");
           gst_tag_list_foreach (tags, printTag, GINT_TO_POINTER (1));
           gst_tag_list_free (tags);
        }
    }*/

    g_print ("\n");
    for (i = 0; i < data->n_audio; i++) 
    {
        tags = NULL;
        /* Retrieve the stream's audio tags */
        g_signal_emit_by_name (data->playbin, "get-audio-tags", i, &tags);
        if (tags) 
	{
            g_print ("Tags:\n");
            gst_tag_list_foreach (tags, printTag, GINT_TO_POINTER (1));
            gst_tag_list_free (tags);
        }
    }


    g_object_get (data->playbin, "current-audio", &data->current_audio, NULL);

    g_print ("\n");
    g_print ("Currently playing audio stream %d \n", data->current_audio);
}


/********************************************************************************************************************
Purpose:               To check the current status of the AV running
Parameters:
scriptname [IN]       - The input scriptname
Return:               - bool SUCCESS/FAILURE
*********************************************************************************************************************/
bool check_for_AV_status ()
{
    GST_LOG ("\nCheck_for_AV_status\n");
    char video_status[BUFFER_SIZE_SHORT] = {'\0'};
    char audio_status[BUFFER_SIZE_SHORT] = {'\0'};

    strcat (video_status, TDK_PATH);
    strcat (video_status, VIDEO_STATUS);
    strcat (audio_status, TDK_PATH);
    strcat (audio_status, AUDIO_STATUS);
  
    /*
     * VideoStatus Check, AudioStatus Check script execution
     */
    return (getstreamingstatus (video_status) && getstreamingstatus (audio_status));
 
}

/********************************************************************************************************************
Purpose:               To read the number of frames rendered from file in DUT
                       File is populated by FRAME_DATA(proc entry file)
Return:               - int number fo frames rendered
*********************************************************************************************************************/
int readFramesFromFile()
{
    int rendered_frames;	
    char rendered_frames_file[BUFFER_SIZE_SHORT] = {'\0'};
    char buffer[BUFFER_SIZE_SHORT]={'\0'};
    char result[BUFFER_SIZE_SHORT]={'\0'};
    strcat (rendered_frames_file, "cat ");
    strcat (rendered_frames_file, TDK_PATH);
    strcat (rendered_frames_file, "/rendered_frames");
    FILE* pipe = popen(rendered_frames_file, "r");
    if (!pipe)
    {
        printf("Error in opening pipe \n");
    }
    while (!feof(pipe))
    {
       if (fgets(buffer, BUFFER_SIZE_SHORT, pipe) != NULL)
       {
            strcat(result, buffer);
       }
    }
    pclose(pipe);
    printf("\nFrames Rendered = %s",result);
    rendered_frames = atoi(result);
    return rendered_frames;
}

/********************************************************************************************************************
Purpose:               To flush the pipeline using seek
*********************************************************************************************************************/
void flushPipeline(GstElement *playbin)
{
    gint64 currentPosition;
    seekOperation = true;
    printf("\nFlushing Pipeline after switch\n");
    assert_failure (playbin,gst_element_query_position (playbin, GST_FORMAT_TIME, &(currentPosition)),
                                         "Failed to query the current playback position");
    seekSeconds = currentPosition/GST_SECOND;
    assert_failure (playbin,gst_element_seek (playbin, NORMAL_PLAYBACK_RATE, GST_FORMAT_TIME,
                                   GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, currentPosition,
                                   GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE), "Failed to seek");
    Sleep(1);
    return;
}

/********************************************************************************************************************
Purpose:               Callback function to set a variable to true on receiving first frame
*********************************************************************************************************************/
static void firstFrameCallback(GstElement *westerosSink, guint size, void *context, gpointer data)
{
   bool *gotFirstFrameSignal = (bool*)data;

   /* Get timestamp of firstFrameRecived to calculate latency */
   if ((latency_check_test) && (!firstFrameReceived))
   {
       timestamp = gst_clock_get_time ((westerosSink)->clock);
       stop_latency = std::chrono::high_resolution_clock::now();
   }

   printf ("Received first frame signal\n");
   /*
    * Set the Value to global variable once the first frame signal is received
    */
   *gotFirstFrameSignal = true;
}

/********************************************************************************************************************
Purpose:               Callback function to set a variable to true on receiving buffer underflow
*********************************************************************************************************************/
static void bufferUnderflowCallback(GstElement *westerosSink, guint size, void *context, gpointer data)
{
   bool *gotVideoUnderflow = (bool*)data;
   gint queued_frames;

   printf ("\nINFO : Received buffer underflow signal from westerossink\n");
   g_object_get (westerosSink,"queued-frames",&queued_frames,NULL);
   printf("\nQueued Frames when underflow is received = %d",queued_frames);
   /*
    * Set the Value to global variable once the first frame signal is received
    */
   *gotVideoUnderflow = true;
}


/********************************************************************************************************************
Purpose:               Method to handle the different messages from gstreamer bus
Parameters:
message [IN]          - GstMessage* handle to the message recieved from bus
data [OUT]	      - MessageHandlerData* handle to the custom structure to pass arguments between calling function
Return:               - None
*********************************************************************************************************************/
static void handleMessage (MessageHandlerData *data, GstMessage *message) 
{
    GError *err;
    gchar *debug_info;
    GstQuery *query;

    switch (GST_MESSAGE_TYPE (message)) 
    {
        case GST_MESSAGE_ERROR:
            gst_message_parse_error (message, &err, &debug_info);
            printf ("Error received from element %s: %s\n", GST_OBJECT_NAME (message->src), err->message);
            printf ("Debugging information: %s\n", debug_info ? debug_info : "none");
            g_clear_error (&err);
            g_free (debug_info);
            data->terminate = TRUE;
            break;
        case GST_MESSAGE_EOS:
            printf ("End-Of-Stream reached.\n");
            data->eosDetected = TRUE;
            data->terminate = TRUE;
            break;
            /*
            * In case of seek event, state change of various gst elements occur asynchronously
            * We can check if the seek event happened in between by querying the current position while ASYNC_DONE message is retrieved
            * If the current position is not updated, we will wait until bus is clear or error/eos occurs
            */
        case GST_MESSAGE_STATE_CHANGED:
            data->stateChanged = TRUE;
        case GST_MESSAGE_ASYNC_DONE:
	    if (data->setRateOperation == TRUE)
            {
               query = gst_query_new_segment (GST_FORMAT_DEFAULT);
               assert_failure (data->playbin,gst_element_query (data->playbin, query), "Failed to query the current playback rate");
               gst_query_parse_segment (query, &(data->currentRate), NULL, NULL, NULL);
               if (data->setRate == data->currentRate)
               {
                   time_elapsed = gst_clock_get_time ((data->playbin)->clock);
                   data->seeked = TRUE;
               }
            }
            else
            {
               assert_failure (data->playbin,gst_element_query_position (data->playbin, GST_FORMAT_TIME, &(data->currentPosition)),
                                                     "Failed to querry the current playback position");
               //Added GST_SECOND buffer time between currentPosition and seekPosition
               if (abs( data->currentPosition - data->seekPosition) <= (GST_SECOND))
               {
                   data->seeked = TRUE;
                   time_elapsed = gst_clock_get_time ((data->playbin)->clock);
               }
            }
            break;
        case GST_MESSAGE_STREAM_START:
            data->streamStart = TRUE;
            break;
        default:
            printf ("Unexpected message received.\n");
            data->terminate = TRUE;
            break;
    }
    gst_message_unref (message);
}

static gboolean print_value (GQuark field, const GValue * value, gpointer pointer)
{
  gchar *string = gst_value_serialize (value);

  printf ("%s  %15s: %s\n", (gchar *) pointer, g_quark_to_string (field), string);
  const char* substr = "bit-depth-luma";
  if (std::strstr(g_quark_to_string (field), substr))
  //if (strcmp ((g_quark_to_string (field), "bit-depth-luma") == 0)
      printf("\nValue got is %s\n",string);
      Bit_depth_got = atoi(string);

  //fail_unless(strcmp ((gchar *) string,(gchar *) bit_depth)==0,"Excepted bitdepth is not received");

  g_free (string);
  return TRUE;
}

static void print_color_parameters (const GstCaps * color_parameters, const gchar * pointer)
{
  guint i;

  g_return_if_fail (color_parameters != NULL);

  if (gst_caps_is_any (color_parameters))
  {
    printf ("%sANY\n", pointer);
    return;
  }
  if (gst_caps_is_empty (color_parameters))
  {
    printf ("%sEMPTY\n", pointer);
    return;
  }

  for (i = 0; i < gst_caps_get_size (color_parameters); i++)
  {
    GstStructure *structure = gst_caps_get_structure (color_parameters, i);

    printf ("%s%s\n", pointer, gst_structure_get_name (structure));
    gst_structure_foreach (structure, print_value, (gpointer) pointer);
  }
}


static void print_pad_capabilities (GstElement *element, gchar *pad_name)
{
  GstPad *pad = NULL;
  GstCaps *color_parameters = NULL;

  /* Retrieve pad */
  pad = gst_element_get_static_pad (element, pad_name);
  if (!pad)
  {
    printf ("Could not retrieve pad '%s'\n", pad_name);
    return;
  }

  /* Retrieve negotiated caps (or acceptable caps if negotiation is not finished yet) */
  color_parameters = gst_pad_get_current_caps (pad);
  if (!color_parameters)
  color_parameters = gst_pad_query_caps (pad, NULL);

  /* Print and free */
  printf ("Caps for the %s pad:\n", pad_name);
  print_color_parameters (color_parameters, "      ");
  gst_caps_unref (color_parameters);
  gst_object_unref (pad);
}

std::string executecommand(GstElement *playbin,const std::string& command )
{
	string resultDetails;

	char readRespBuff[BUFF_LENGTH] = { '\0' };

	FILE *file = popen (command.c_str(), "r");

	assert_failure (playbin,file != NULL,"Unable to read file");


	while(fgets(readRespBuff,sizeof(readRespBuff),file) != NULL)
	{
		resultDetails += readRespBuff;
	}

	pclose(file);
	return resultDetails;

}




/********************************************************************************************************************
 * Purpose     	: Test to do basic initialisation and shutdown of gst pipeline with playbin element and westeros-sink
 * Parameters   : Playback url
 ********************************************************************************************************************/
GST_START_TEST (test_init_shutdown)
{
    GstElement *playbin;
    GstElement *westerosSink;

    /*
     * Create the playbin element
     */
    playbin = gst_element_factory_make(PLAYBIN_ELEMENT, NULL);
    fail_unless (playbin != NULL, "Failed to create 'playbin' element");
    /*
     * Set the url received from argument as the 'uri' for playbin
     */
    assert_failure (playbin,m_play_url != NULL, "Playback url should not be NULL"); 
    g_object_set (playbin, "uri", m_play_url, NULL);
    /*
     * Update the current playbin flags to enable Video and Audio Playback
     */
    g_object_get (playbin, "flags", &flags, NULL);
    setflags();
    g_object_set (playbin, "flags", flags, NULL);

    /*
     * Set westros-sink
     */
    westerosSink = gst_element_factory_make(WESTEROS_SINK, NULL);
    fail_unless (westerosSink != NULL, "Failed to create 'westerossink' element");

    /*
     * Link the westeros-sink to playbin
     */
    g_object_set (playbin, "video-sink", westerosSink, NULL);

    if (playbin)
    {
       gst_element_set_state (playbin, GST_STATE_NULL);
    }
    /*
     * Cleanup after use
     */
    gst_object_unref (playbin);
}
GST_END_TEST;

/********************************************************************************************************************
 * Purpose      : Test to calculate the playback latency from westeros sink to display renderer
 * Parameters   : Playback url
 ********************************************************************************************************************/
GST_START_TEST (test_playback_latency)
{
    bool is_av_playing = false;
    GstElement *playbin;
    GstElement *westerosSink;
    GstState cur_state;
    latency_check_test = true;

    /*
     * Create the playbin element
     */
    playbin = gst_element_factory_make(PLAYBIN_ELEMENT, NULL);
    fail_unless (playbin != NULL, "Failed to create 'playbin' element");
    /*
     * Set the url received from argument as the 'uri' for playbin
     */
    assert_failure (playbin,m_play_url != NULL, "Playback url should not be NULL");
    g_object_set (playbin, "uri", m_play_url, NULL);
    /*
     * Update the current playbin flags to enable Video and Audio Playback
     */
    g_object_get (playbin, "flags", &flags, NULL);
    setflags();
    g_object_set (playbin, "flags", flags, NULL);

    /*
     * Set video-sink
     */
    westerosSink = gst_element_factory_make(WESTEROS_SINK, NULL);
    fail_unless (westerosSink != NULL, "Failed to create 'westerossink' element");
    g_signal_connect( westerosSink, "first-video-frame-callback", G_CALLBACK(firstFrameCallback), &firstFrameReceived);
    g_object_set (playbin, "video-sink", westerosSink, NULL);
 
    auto start_latency = std::chrono::high_resolution_clock::now();
    start = std::chrono::steady_clock::now();
    bool gotLatencyStart = false;
    assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE);
    while(1)
    {
        //Run loop maximum for 5 seconds
        if ( std::chrono::steady_clock::now() - start > std::chrono::seconds(5) )
          break;
        // break if firstFrame is received
        if (firstFrameReceived)
          break;
        gst_element_get_state (playbin, &cur_state, NULL, 0);
        if ((cur_state == GST_STATE_PLAYING) && (gotLatencyStart == false))
        {
          start_latency = std::chrono::high_resolution_clock::now();
          gotLatencyStart = true;
        }

    }

    assert_failure (playbin,firstFrameReceived == true, "Failed to receive first video frame signal");  
    auto New_latency = std::chrono::duration_cast<std::chrono::milliseconds>(stop_latency - start_latency);
    printf("\nTime measured: %.3lld milliseconds.\n", New_latency.count());

    int latency_int = New_latency.count();
    /*
     * Writing to file
     */
    FILE *filePointer ;
    char latency_file[BUFFER_SIZE_SHORT] = {'\0'};
    strcat (latency_file, TDK_PATH);
    strcat (latency_file, "/latency_log");
    filePointer = fopen(latency_file, "w");
    if (filePointer != NULL)
    {
        fprintf(filePointer,"Latency = %d milliseconds\n", latency_int);
    }
    else
    {
	printf("\nLatency writing operation failed\n");
    }
    fclose(filePointer);

    Sleep(2);

    PlaySeconds(playbin,5);
    if (playbin)
    {
       gst_element_set_state (playbin, GST_STATE_NULL);
    }
    /*
     * Cleanup after use
     */
    gst_object_unref (playbin);
}
GST_END_TEST;    
   


/********************************************************************************************************************
 * Purpose     	: Test to do generic playback using playbin element and westeros-sink
 * Parameters   : Playback url
 ********************************************************************************************************************/
GST_START_TEST (test_generic_playback)
{
    bool is_av_playing = false;
    GstElement *playbin;
    GstElement *westerosSink;
    GstElement *rialtovsink, *rialtoasink;
    GstMessage *message;
    GstBus *bus;
    MessageHandlerData data;

    /*
     * Create the playbin element
     */
    playbin = gst_element_factory_make(PLAYBIN_ELEMENT, NULL);
    fail_unless (playbin != NULL, "Failed to create 'playbin' element");
    /*
     * Set the url received from argument as the 'uri' for playbin
     */
    assert_failure (playbin,m_play_url != NULL, "Playback url should not be NULL"); 
    g_object_set (playbin, "uri", m_play_url, NULL);
    /*
     * Update the current playbin flags to enable Video and Audio Playback
     */
    g_object_get (playbin, "flags", &flags, NULL);
    setflags();
    g_object_set (playbin, "flags", flags, NULL);

    /*
     * Set video-sink
     */
    if (use_rialto)
    {
       rialtovsink = gst_element_factory_make("rialtomsevideosink", NULL);
       assert_failure (playbin,rialtovsink != NULL, "Failed to create 'rialtomsevideosink' element");

       rialtoasink = gst_element_factory_make("rialtomseaudiosink", NULL);
       assert_failure (playbin,rialtovsink != NULL, "Failed to create 'rialtomseaudiosink' element");

       g_object_set (playbin, "audio-sink", rialtoasink, NULL);
    }
    else
    {
        westerosSink = gst_element_factory_make(WESTEROS_SINK, NULL);
	fail_unless (westerosSink != NULL, "Failed to create 'westerossink' element");
    }

    if(use_rialto)
    {
	g_object_set (playbin, "video-sink", rialtovsink, NULL);
    }
    else
    {
	g_object_set (playbin, "video-sink", westerosSink, NULL);
    }

    if (!use_rialto)
    {
       /*
        * Set the first frame recieved callback
        */
        g_signal_connect( westerosSink, "first-video-frame-callback", G_CALLBACK(firstFrameCallback), &firstFrameReceived);
       /*
        * Set the firstFrameReceived variable as false before starting play
        */
        firstFrameReceived= false;
    }

    /*
     * Set playbin to PLAYING
     */
    GST_FIXME( "Setting to Playing State\n");
    assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE,"Failed to set to playing state");
    GST_FIXME( "Set to Playing State\n");

    WaitForOperation;
    /*
     * Check if the first frame received flag is set
     */
    if (!use_rialto)
        assert_failure (playbin,firstFrameReceived == true, "Failed to receive first video frame signal");

    /*
     * Wait for 'play_timeout' seconds(recieved as the input argument) before checking AV status
     */

    if (useProcForFPS)
    {
         char frame_status[BUFFER_SIZE_SHORT] = {'\0'};
         strcat (frame_status, TDK_PATH);
         strcat (frame_status, FRAME_DATA);
         strcat (frame_status, " &");
	 system(frame_status);
    }

    data.playbin = playbin;
    if (use_rialto)
    {
	 g_object_get (rialtovsink, "maxVideoHeight", &height, NULL);
	 g_object_get (rialtovsink, "maxVideoWidth", &width, NULL);
    }
    else
    {
         g_object_get (westerosSink, "video-height", &height, NULL);
         g_object_get (westerosSink, "video-width", &width, NULL);
    }

    printf("\nVideo height = %d\nVideo width = %d", height, width);

    int n_audio;
    g_object_get (playbin, "n-audio", &n_audio, NULL);

    if (n_audio != 0)
    {
        printf("\nAudio is present in the video.\n");
    }
    else
    {
        printf("\nAudio is not present in the video.\nSkipping Audio FPS check\n");
	use_audioSink = false;
    }

    if (ResolutionTest)
    {
	 if (!resolution.empty())
	 {
	     printf("\nChecking if video is playing at %s",resolution.c_str());
	     resolution.pop_back();
             char resolution_value_string[150];
	     sprintf(resolution_value_string,"\nPipeline is not playing at expected resolution\nObtained video-height as %d and video-width as %d",height,width);
	     assert_failure (playbin,to_string(height) == resolution,resolution_value_string);
             PlaySeconds(playbin,play_timeout);
	     printf("\nSUCCESS : Resolution is set to %sp successfully\n",resolution.c_str());
	 }
    }
    else
    {
	 PlaySeconds(playbin,play_timeout);
    }

    if (ResolutionSwitchTest)
    {
	 assert_failure (playbin,ResolutionCount == TOTAL_RESOLUTIONS_COUNT,"\nNot able to play all resolutions\n");
    }
    if (useProcForFPS)
    {
	int rendered_frames;
	rendered_frames = readFramesFromFile();
        printf("\nRendered Frames %d\n",rendered_frames);
    }
    /*
     * Calculate latency by obtaining the sink base time
     */
    if (latency_check_test)
    {
         latency = timestamp - gst_element_get_base_time (GST_ELEMENT_CAST (westerosSink));
	 parselatency();
    }
    /*
     * Check for AV status if its enabled
     */
    if (true == checkAVStatus)
    {
        is_av_playing = check_for_AV_status();
        assert_failure (playbin,is_av_playing == true, "Video is not playing in TV");
    }
    GST_LOG("DETAILS: SUCCESS, Video playing successfully \n");

    if (playbin)
    {
       assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_NULL) !=  GST_STATE_CHANGE_FAILURE,"Failed to set to NULL state");
    }
    if (ChannelChangeTest)
    {
       /*
        * Set the url received from argument as the 'channel_url' for playbin
        */
       assert_failure (playbin,channel_url != NULL, "Second Channel url should not be NULL");
       g_object_set (playbin, "uri", channel_url, NULL);

       /*
        * Set all the required variables before polling for the message
        */
       data.streamStart= FALSE;
       data.terminate = FALSE;
       data.playbin = playbin;

       printf("\nLoading Second Channel\n");

       /*
        * Set playbin to PLAYING
        */
       GST_LOG( "Setting Second Channel to Playing State\n");
       assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE,"Failed to set to playing state");
       GST_LOG( "Second Channel Set to Playing State\n");
       
       bus = gst_element_get_bus (playbin);
       do
       {
           message = gst_bus_timed_pop_filtered(bus, 2 * GST_SECOND,(GstMessageType)((GstMessageType) GST_MESSAGE_STREAM_START|(GstMessageType) GST_MESSAGE_ERROR | (GstMessageType) GST_MESSAGE_EOS ));
           if (NULL != message)
           {
               handleMessage (&data, message);
           }
           else
           {
               printf ("All messages are clear. No more message after seek\n");
               break;
           }
       }while(!data.streamStart && !data.terminate);

       Sleep(2); 

       if ( (TRUE == data.terminate) || ( FALSE == data.streamStart))
	    gst_object_unref(bus);

       /*
        * Verify that ERROR/EOS messages are not recieved
        */
       assert_failure (playbin,FALSE == data.terminate, "Unexpected error or End of Stream recieved\n");
       /*
        * Verify that STREAM_START message is received
        */
       assert_failure (playbin,TRUE == data.streamStart, "Unable to obtain message indicating start of a new stream\n");
       /*
        * Check for AV status if its enabled
        */
       if (true == checkAVStatus)
       {
          is_av_playing = check_for_AV_status();
          assert_failure (playbin,is_av_playing == true, "Video is not playing in TV");
       }
       GST_LOG("DETAILS: SUCCESS, Video playing successfully \n");
       /*
        * Wait for 'SecondChannelTimeout' seconds(received as the input argument) for video to play
        */
       PlaySeconds(playbin,SecondChannelTimeout);

       if (playbin)
       {
          gst_element_set_state (playbin, GST_STATE_NULL);
       }
       gst_object_unref(bus);
    }
    /*
     * Cleanup after use
     */
    gst_object_unref (playbin);
}
GST_END_TEST;

/********************************************************************************************************************
 * Purpose      : Test to change the state of pipeline from play to pause
 * Parameters   : Playback url
 ********************************************************************************************************************/
GST_START_TEST (test_play_pause_pipeline)
{
    bool is_av_playing = false;
    GstState cur_state;
    GstElement *playbin;
    GstElement *westerosSink, *rialtovsink, *rialtoasink;
    GstStateChangeReturn state_change;
    bool paused = false;
    GstMessage *message;
    GstBus *bus;
    gint TIME_TO_PAUSE = 2000;
    
    /*
     * Create the playbin element
     */
    playbin = gst_element_factory_make(PLAYBIN_ELEMENT, NULL);
    fail_unless (playbin != NULL, "Failed to create 'playbin' element");

    /*
     * Set the url received from argument as the 'uri' for playbin
     */
    assert_failure (playbin,m_play_url != NULL, "Playback url should not be NULL");
    g_object_set (playbin, "uri", m_play_url, NULL);

    /*
     * Update the current playbin flags to enable Video and Audio Playback
     */
    g_object_get (playbin, "flags", &flags, NULL);
    setflags();
    g_object_set (playbin, "flags", flags, NULL);

    /*
     * Create videosink instance
     */
    if (use_rialto)
    {
       rialtovsink = gst_element_factory_make("rialtomsevideosink", NULL);
       assert_failure (playbin,rialtovsink != NULL, "Failed to create 'rialtomsevideosink' element");

       rialtoasink = gst_element_factory_make("rialtomseaudiosink", NULL);
       assert_failure (playbin,rialtovsink != NULL, "Failed to create 'rialtomseaudiosink' element");

       g_object_set (playbin, "audio-sink", rialtoasink, NULL);
    }
    else
    {
       westerosSink = gst_element_factory_make(WESTEROS_SINK, NULL);
       fail_unless (westerosSink != NULL, "Failed to create 'westerossink' element");
    }

    if(use_rialto)
    {
        g_object_set (playbin, "video-sink", rialtovsink, NULL);
    }
    else
    {
        g_object_set (playbin, "video-sink", westerosSink, NULL);
    }

    if (!use_rialto)
    {
       /*
        * Set the first frame recieved callback
        */
        g_signal_connect( westerosSink, "first-video-frame-callback", G_CALLBACK(firstFrameCallback), &firstFrameReceived);
       /*
        * Set the firstFrameReceived variable as false before starting play
        */
        firstFrameReceived= false;
    }

    /*
     * Set playbin to PLAYING
     */
    GST_FIXME( "Setting to Playing State\n");
    assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "Unable to pause");
    GST_FIXME( "Set to Playing State\n");

    WaitForOperation;

    if (!use_rialto)
    {
       /*
        * Check if the first frame received flag is set
        */
        assert_failure (playbin,firstFrameReceived == true, "Failed to receive first video frame signal");
    }

    /*
     * Wait for 'play_timeout' seconds(recieved as the input argument) before changing the pipeline state
     * We are waiting for 5 more seconds before checking pipeline status, so reducing the wait here
     */
    PlaySeconds(playbin,play_timeout);


    if (use_rialto)
    {
         g_object_get (rialtovsink, "maxVideoHeight", &height, NULL);
         g_object_get (rialtovsink, "maxVideoWidth", &width, NULL);
    }
    else
    {
         g_object_get (westerosSink, "video-height", &height, NULL);
         g_object_get (westerosSink, "video-width", &width, NULL);
    }
    printf("\nVideo height = %d\nVideo width = %d", height, width);

    /*
     * Ensure that playback is happening before pausing the pipeline
     */

    /*
     * Check for AV status if its enabled
     */
    if (true == checkAVStatus)
    {
        is_av_playing = check_for_AV_status();
        assert_failure (playbin,is_av_playing == true, "Video is not playing in TV");
    }
    printf ("\nDETAILS: SUCCESS, Video playing successfully\n");

    auto latency_start = std::chrono::high_resolution_clock::now();
    auto latency_stop =  std::chrono::high_resolution_clock::now();
    bus = gst_element_get_bus (playbin);
    auto latency_chrono = std::chrono::duration_cast<std::chrono::milliseconds>(latency_stop - latency_start);

    /*
     * Set pipeline to PAUSED
     */
    gst_element_set_state (playbin, GST_STATE_PAUSED);

    /*
     * Wait for 5 seconds before checking the pipeline status
     */
    latency_start = std::chrono::high_resolution_clock::now();
    do
    {
	message = gst_bus_pop_filtered (bus,(GstMessageType)GST_MESSAGE_STATE_CHANGED);
        if (message != NULL)
        {
            if (GST_MESSAGE_STATE_CHANGED == GST_MESSAGE_TYPE(message))
            {
		gst_element_get_state (playbin, &cur_state, NULL, 0);
		if (cur_state == GST_STATE_PAUSED)
		{
		    latency_stop =  std::chrono::high_resolution_clock::now();
		    latency_chrono = std::chrono::duration_cast<std::chrono::milliseconds>(latency_stop - latency_start);
		    printf("\nTime taken to PAUSE: %.3lld milliseconds.\n", latency_chrono.count());
		    paused = true;
		}
	    }
        }
	if (std::chrono::high_resolution_clock::now() - latency_start > std::chrono::seconds(PAUSE_TIMEOUT))
            break;
    }while(!paused);

    gst_object_unref(bus);

    assert_failure (playbin,paused == true, "Unable to pause pipeline");
    //char latency_value_string[100];
    //sprintf(latency_value_string,"Time taken to PAUSE is greater than 2 seconds, Actual PAUSE latency %.3lld milliseconds",latency_chrono.count());
    assert_failure (playbin,(int)latency_chrono.count() < TIME_TO_PAUSE,"Time taken to PAUSE is greater than 2 seconds");
    PlaySeconds(playbin,play_timeout);

    GST_LOG("\n********Current state: %s\n",gst_element_state_get_name(cur_state));
    assert_failure (playbin, cur_state == GST_STATE_PAUSED);
    printf ("DETAILS: SUCCESS, Current state is: %s \n", gst_element_state_get_name(cur_state));

    assert_failure (playbin, gst_element_get_state (playbin, &cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS);
    if ((cur_state != GST_STATE_PLAYING))
    {
        assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "unable to pause");
        do{
             state_change = gst_element_get_state (playbin, &cur_state, NULL, 10000000);
        } while (state_change == GST_STATE_CHANGE_ASYNC);
	printf ("\n********Current state is: %s \n", gst_element_state_get_name(cur_state));
    }

    PlaySeconds(playbin,5);


    if (playbin)
    {
       gst_element_set_state(playbin, GST_STATE_NULL);
    }

    /*
     * Cleanup after use
     */
    gst_object_unref (playbin);
}
GST_END_TEST;

/********************************************************************************************************************
 * Purpose      : Test to verify if buffer underflow signal is obtained upon videounderrun
                  and playabck is smooth after video buffer is full again
 * Parameters   : Playback url
 ********************************************************************************************************************/
GST_START_TEST (test_buffer_underflow)
{
    bool is_av_playing = false;
    GstState cur_state;
    GstElement *playbin;
    GstElement *westerosSink;
    gint64 seekPosition;
    gint64 currentPosition;
    bool seeked = false;
    int Seek_time_threshold = 5;
    GstStateChangeReturn state_change;

    /*
     * Create the playbin element
     */
    playbin = gst_element_factory_make(PLAYBIN_ELEMENT, NULL);
    fail_unless (playbin != NULL, "Failed to create 'playbin' element");
    /*
     * Set the url received from argument as the 'uri' for playbin
     */
    assert_failure (playbin,m_play_url != NULL, "Playback url should not be NULL");
    g_object_set (playbin, "uri", m_play_url, NULL);
    /*
     * Update the current playbin flags to enable Video and Audio Playback
     */
    g_object_get (playbin, "flags", &flags, NULL);
    setflags();
    g_object_set (playbin, "flags", flags, NULL);
    /*
     * Create westerosSink instance
     */
    westerosSink = gst_element_factory_make(WESTEROS_SINK, NULL);
    fail_unless (westerosSink != NULL, "Failed to create 'westerossink' element");
    g_object_set (playbin, "video-sink", westerosSink, NULL);

    /*
     * Set the first frame recieved callback and buffer underflow call back
     */
    g_signal_connect( westerosSink, "first-video-frame-callback", G_CALLBACK(firstFrameCallback), &firstFrameReceived);
    g_signal_connect( westerosSink, "buffer-underflow-callback", G_CALLBACK(bufferUnderflowCallback), &videoUnderflowReceived);

    /*
     * Set the firstFrameReceived and videoUnderflowReceived variable as false before starting play
     */
    firstFrameReceived= false;
    videoUnderflowReceived= false;
    /*
     * Set playbin to PLAYING
     */
    GST_FIXME( "Setting to Playing State\n");
    assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "unable to pause");
    GST_FIXME( "Set to Playing State\n");
    WaitForOperation;

    /*
     * Check if the first frame received flag is set
     */
    assert_failure (playbin,firstFrameReceived == true, "Failed to receive first video frame signal");

    assert_failure (playbin,videoEnd != 0,"videoEnd point is not given");
    if (play_without_video)
    {
	 justPrintPTS = true;
	 bufferUnderflowTest = false;
	 ignorePlayJump = true;
	 use_audioSink = true;

	 /*
	  * Play the pipeline even when video isn't available in buffer
	  * That is pipeline should play audio alone
	  */
	 PlaySeconds(playbin,play_timeout + videoEnd);
    }
    else
    {
	 /*
	  * Wait for 'videoEnd' seconds(recieved as the input argument) to receive underflow signal from westerossink
	  */
	 PlaySeconds(playbin,videoEnd);
    }

    assert_failure (playbin,videoUnderflowReceived == true, "Failed to receive buffer underflow signal");
    printf ("\nDETAILS: SUCCESS, Received buffer underflow signal as expected\n");

    /*
     * if checkSignalTest or play_without_video is enabled
     * Check only if buffer underflow signal is received
     */
    if (checkSignalTest || play_without_video)
	 goto EXIT;

    /*
     * Set the playbin state to GST_STATE_PAUSED
     */
    bufferUnderflowTest = false;

    gst_element_set_state (playbin, GST_STATE_PAUSED);
    PlaySeconds(playbin,5);
    assert_failure (playbin, gst_element_get_state (playbin, &cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS);
    assert_failure (playbin, cur_state == GST_STATE_PAUSED);
    printf("\n********Current state: %s\n",gst_element_state_get_name(cur_state));
    GST_LOG("\n********Current state: %s\n",gst_element_state_get_name(cur_state));

    /*
     * Seek to the 'videoStart' point (recieved as the input argument) to fill the video buffer
     * And start playback
     */
    assert_failure (playbin,videoStart != 0,"videoStart point is not given");
    assert_failure (playbin,videoStart > videoEnd,"videoEnd and videoStart params are not correct");
    seekPosition = GST_SECOND * (videoStart + 2);
    seekOperation = true;
    seekSeconds = seekPosition/GST_SECOND;
    assert_failure (playbin,gst_element_seek (playbin, NORMAL_PLAYBACK_RATE, GST_FORMAT_TIME,
                                   GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, seekPosition,
                                   GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE), "Failed to seek");

    start = std::chrono::steady_clock::now();
    while(!seeked)
    {
       //Check if seek happened
       assert_failure (playbin,gst_element_query_position (playbin, GST_FORMAT_TIME, &(currentPosition)),
                                         "Failed to query the current playback position");
       //Added GST_SECOND buffer time between currentPosition and seekPosition
       if (abs( currentPosition - seekPosition) <= (GST_SECOND))
       {
           seeked = TRUE;
       }
       if (std::chrono::steady_clock::now() - start > std::chrono::seconds(Seek_time_threshold))
           break;
    }

    assert_failure (playbin,TRUE == seeked, "Seek Unsuccessfull\n");
    //Convert time to seconds
    currentPosition /= GST_SECOND;
    seekPosition /= GST_SECOND;
    printf("\nSEEK SUCCESSFULL :  CurrentPosition %lld seconds, SeekPosition %lld seconds\n", currentPosition, seekPosition);

    state_change = gst_element_get_state (playbin, &cur_state, NULL, GST_SECOND);
    assert_failure (playbin,state_change != GST_STATE_CHANGE_FAILURE, "Failed to get current playbin state");

    if ((cur_state == GST_STATE_PAUSED)  && (state_change != GST_STATE_CHANGE_ASYNC))
    {

        /*
         * Set playbin to PLAYING
         */
         GST_FIXME( "Setting to Playing State\n");
         assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "unable to pause");
         GST_FIXME( "Set to Playing State\n");
         do{
              //Waiting for state change
              state_change = gst_element_get_state (playbin, &cur_state, NULL, 10000000);
         } while (state_change == GST_STATE_CHANGE_ASYNC);

         printf("\n********Current state: %s\n",gst_element_state_get_name(cur_state));
	 assert_failure (playbin, gst_element_get_state (playbin, &cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS);
         assert_failure (playbin, cur_state == GST_STATE_PLAYING);
         GST_LOG("\n********Current state: %s\n",gst_element_state_get_name(cur_state));
    }

    PlaySeconds(playbin,play_timeout);
    printf ("\nDETAILS: SUCCESS Playback after seeking to Video Point after buffer underflow is successfull\n");
EXIT :
    /* Check for AV status if its enabled
     */
    if (true == checkAVStatus)
    {
        is_av_playing = check_for_AV_status();
        assert_failure (playbin,is_av_playing == true, "Video is not playing in TV");
    }
    if (playbin)
    {
       gst_element_set_state(playbin, GST_STATE_NULL);
    }
    /*
     * Cleanup after use
     */
    gst_object_unref (playbin);
}
GST_END_TEST;


/********************************************************************************************************************
 * Purpose      : Test to check that EOS message is recieved
 * Parameters   : Playback url
 ********************************************************************************************************************/
GST_START_TEST (test_EOS)
{
    bool is_av_playing = false;
    GstMessage *message;
    use_westerossink_fps = false;    
    GstBus *bus;
    bool received_EOS = false;
    GstElement *playbin;
    GstElement *westerosSink, *rialtovsink, *rialtoasink;

    /*
     * Create the playbin element
     */
    playbin = gst_element_factory_make(PLAYBIN_ELEMENT, NULL);
    fail_unless (playbin != NULL, "Failed to create 'playbin' element");

    /*
     * Set the url received from argument as the 'uri' for playbin
     */
    assert_failure (playbin,m_play_url != NULL, "Playback url should not be NULL");
    g_object_set (playbin, "uri", m_play_url, NULL);

    /*
     * Update the current playbin flags to enable Video and Audio Playback
     */
    g_object_get (playbin, "flags", &flags, NULL);
    setflags();
    g_object_set (playbin, "flags", flags, NULL);

    /*
     * Set video-sink
     */
    if (use_rialto)
    {
       rialtovsink = gst_element_factory_make("rialtomsevideosink", NULL);
       assert_failure (playbin,rialtovsink != NULL, "Failed to create 'rialtomsevideosink' element");

       rialtoasink = gst_element_factory_make("rialtomseaudiosink", NULL);
       assert_failure (playbin,rialtovsink != NULL, "Failed to create 'rialtomseaudiosink' element");

       g_object_set (playbin, "audio-sink", rialtoasink, NULL);
    }
    else
    {
        westerosSink = gst_element_factory_make(WESTEROS_SINK, NULL);
	fail_unless (westerosSink != NULL, "Failed to create 'westerossink' element");
    }

    if(use_rialto)
    {
	g_object_set (playbin, "video-sink", rialtovsink, NULL);
    }
    else
    {
	g_object_set (playbin, "video-sink", westerosSink, NULL);
    }

    if (!use_rialto)
    {
       /*
        * Set the first frame recieved callback
        */
        g_signal_connect( westerosSink, "first-video-frame-callback", G_CALLBACK(firstFrameCallback), &firstFrameReceived);
       /*
        * Set the firstFrameReceived variable as false before starting play
        */
        firstFrameReceived= false;
    }

    /*
     * Set playbin to PLAYING
     */
    GST_FIXME( "Setting to Playing State\n");
    assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "unable to pause");
    GST_FIXME( "Set to Playing State\n");
    WaitForOperation;

    if (!use_rialto)
    {
       /*
        * Check if the first frame received flag is set
        */
        assert_failure (playbin,firstFrameReceived == true, "Failed to receive first video frame signal");
    }

    /*
     * Wait for 5 seconds before checking AV status
     */
    PlaySeconds (playbin,5);
    /*
     * Check for AV status if its enabled
     */
    if (true == checkAVStatus)
    {
        is_av_playing = check_for_AV_status();
        assert_failure (playbin,is_av_playing == true, "Video is not playing in TV");
    }
    printf ("DETAILS: SUCCESS, Video playing successfully \n");

    /*
     * Polling the bus for messages
     */
    bus = gst_element_get_bus (playbin);
    assert_failure (playbin,bus != NULL,"Failed to get bus");
    /* 
     * Wait for receiving EOS event
     */
    start = std::chrono::steady_clock::now();
    do
    {
	message = gst_bus_pop_filtered (bus,(GstMessageType)GST_MESSAGE_EOS);
        if (message != NULL)
        {
            if (GST_MESSAGE_EOS == GST_MESSAGE_TYPE(message))
                received_EOS = true;
        }
	if (std::chrono::steady_clock::now() - start > std::chrono::seconds(EOS_TIMEOUT))
            break;
    }while(!received_EOS);

    gst_object_unref(bus);
    assert_failure (playbin,received_EOS == true, "Failed to recieve EOS message");
    printf ("EOS Received\n");
    gst_message_unref (message);


    if (playbin)
    {
       gst_element_set_state(playbin, GST_STATE_NULL);
    }
    /*
     * Cleanup after use
     */
    gst_object_unref (playbin);
}
GST_END_TEST;


GST_START_TEST (test_frameDrop)
{
    GstElement *pipeline, *videoSink ,*westerosSink;
    gchar *fps_msg;
    gint jump_buffer = 3;
    gint play_jump = 0;
    guint64 previous_rendered_frames, rendered_frames, dropped_frames_video, previous_rendered_frames_proc;
    int expected_rendered_frames;
    int dropped_frames;
    gint frame_buffer = 3;
    gint64 currentPosition;
    gint previous_position = 0;
    gint64 startPosition;
    gint difference;
    GstStructure *structure;
    GstMessage *message;
    GstBus *bus;
    bool runLoop = false;


    pipeline = gst_element_factory_make ("playbin", "source");
    fail_unless (pipeline != NULL, "Failed to create 'pipeline' element");


    westerosSink = gst_element_factory_make(WESTEROS_SINK, NULL);
    fail_unless (westerosSink != NULL, "Failed to create 'westerossink' element");


    g_object_set (pipeline, "video-sink",westerosSink, NULL);
    g_object_set (pipeline, "uri", m_play_url, NULL);


    g_signal_connect( westerosSink, "first-video-frame-callback", G_CALLBACK(firstFrameCallback), &firstFrameReceived);


    GST_FIXME( "Setting to Playing State\n");
    assert_failure (pipeline,gst_element_set_state (pipeline, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE);
    GST_FIXME( "Set to Playing State\n");

    WaitForOperation;

    assert_failure (pipeline,firstFrameReceived == true, "Failed to receive first video frame signal");

    assert_failure (pipeline,gst_element_query_position (pipeline, GST_FORMAT_TIME, &currentPosition), "Failed to query the current playback position");
    
    previous_position = (currentPosition/GST_SECOND);
    if (useProcForFPS)
    {
	 char frame_status[BUFFER_SIZE_SHORT] = {'\0'};
         strcat (frame_status, "sh ");
         strcat (frame_status, TDK_PATH);
         strcat (frame_status, FRAME_DATA);
	 strcat (frame_status, " &");
	 system(frame_status);
    }

    bus = gst_element_get_bus (pipeline);
    if (use_westerossink_fps)
    {
        g_object_get (westerosSink,"stats",&structure,NULL);
        gst_structure_get_uint64(structure, "rendered", &rendered_frames);
        previous_rendered_frames = rendered_frames;
    }
    if (useProcForFPS)
    {
        previous_rendered_frames_proc = readFramesFromFile();
    }


    do
    {
        Sleep(1);
        runLoop = false;
        if (use_westerossink_fps)
        {
            g_object_get(westerosSink,"stats",&structure,NULL);

            if (structure && (gst_structure_has_field(structure, "dropped") || gst_structure_has_field(structure, "rendered")))
            {
                gst_structure_get_uint64(structure, "dropped", &dropped_frames_video);
                gst_structure_get_uint64(structure, "rendered", &rendered_frames);
                printf("\n\nDropped_Frames: %" G_GUINT64_FORMAT, dropped_frames_video);
                printf("\nRendered_Frames: %" G_GUINT64_FORMAT, rendered_frames);
            }
	    if ((rendered_frames <= previous_rendered_frames))
                frame_buffer -= 1;
	    if (frame_buffer == 0)
		gst_object_unref(bus);
            assert_failure (pipeline,frame_buffer != 0 , "frames are not rendered properly");
        }

	runLoop = gst_element_query_position (pipeline, GST_FORMAT_TIME, &currentPosition);
	if (!runLoop)
	    gst_object_unref(bus);
        assert_failure (pipeline,runLoop, "Failed to query the current playback position");
	runLoop = false;

        difference = int(abs((currentPosition/GST_SECOND) - (startPosition/GST_SECOND)));
        printf("\nCurrent Position : %lld , Playing after operation for: %d",(currentPosition/GST_SECOND),difference);

        play_jump = int(currentPosition/GST_SECOND) - previous_position;
        printf("\nPlay jump = %d", play_jump);

        if (play_jump != NORMAL_PLAYBACK_RATE)
            jump_buffer -=1;

        if (!checkTotalFrames)
	{
	    if (rendered_frames < previous_rendered_frames)
	        gst_object_unref(bus);	    
            assert_failure (pipeline,rendered_frames > previous_rendered_frames, "Frames not rendered properly");
	}


	if ((checkTotalFrames) && ((totalFrames - rendered_frames) < MIN_FRAMES_DROP))
            break;

        if (!checkTotalFrames)
	{
	    if (jump_buffer==0)
                gst_object_unref(bus);
            assert_failure (pipeline, jump_buffer != 0 , "Playback is not happening at the expected rate");
	}


        message = gst_bus_pop_filtered (bus,(GstMessageType)GST_MESSAGE_EOS);
        if (message != NULL)
        {
            if (GST_MESSAGE_EOS == GST_MESSAGE_TYPE(message))
            {
                printf("\nEOS Received:Exiting\n");
                break;
            }
        }


        if (!checkTotalFrames)
        {
            if ((currentPosition/GST_SECOND) < play_timeout)
                runLoop = true;
            else
                runLoop = false;
        }
	else
	{
	    if (useProcForFPS)
		rendered_frames = readFramesFromFile();
            if(rendered_frames < previous_rendered_frames_proc)
                jump_buffer -=1;
            if(jump_buffer == 0)
                runLoop = false;
            else
                runLoop = true;
	}
        previous_rendered_frames_proc = rendered_frames;
	previous_rendered_frames = rendered_frames;
        previous_position = (currentPosition/GST_SECOND);

    }while(runLoop);

    gst_object_unref (bus);
    assert_failure (pipeline,gst_element_set_state (pipeline, GST_STATE_PAUSED) !=  GST_STATE_CHANGE_FAILURE, "Unable to pause");
    Sleep(1);

    if (checkTotalFrames)
    {
    expected_rendered_frames = totalFrames;
    }
    else
    {
        expected_rendered_frames = fps * play_timeout;
    }
    printf("\nExpected rendered frames : %d", expected_rendered_frames);



    if (rendered_frames < expected_rendered_frames)
    {
         dropped_frames = expected_rendered_frames - rendered_frames;
	 printf("\n\ndropped_frames : %d", dropped_frames);
	 sleep(2);
	 if (dropped_frames)
	     assert_failure(pipeline,dropped_frames < MIN_FRAMES_DROP,"\nExpected number of frames not rendered\n");
    }
    printf ("\nExecution was Success\n");




    if (pipeline)
    {
       gst_element_set_state(pipeline, GST_STATE_NULL);
    }

    gst_object_unref (pipeline);
}
GST_END_TEST;



/********************************************************************************************************************
 * Purpose      : Test to do audio change during playback using playbin element and westeros-sink
 * Parameters   : Audio Change during playback
 ********************************************************************************************************************/
GST_START_TEST (test_audio_change)
{
    bool is_av_playing = false;
    bool elementsetup = false;
    GstElement *playbin;
    GstElement *westerosSink;
    use_westerossink_fps = false;
    MessageHandlerData data;
    char audio_switch_error_string[100];
    GstTagList *tags;
    strcat (audio_change_test, TDK_PATH);
    strcat (audio_change_test, "/audio_change_log");
    filePointer = fopen(audio_change_test, "w");
    assert_failure (playbin,filePointer != NULL,"Unable to open filePointer for writing");
     
    /*
     * Create the playbin element
     */
    playbin = gst_element_factory_make(PLAYBIN_ELEMENT, NULL);
    fail_unless (playbin != NULL, "Failed to create 'playbin' element");
    /*
     * Set the url received from argument as the 'uri' for playbin
     */
    assert_failure (playbin,m_play_url != NULL, "Playback url should not be NULL");
    g_object_set (playbin, "uri", m_play_url, NULL);
    /*
     * Update the current playbin flags to enable Video and Audio Playback
     */
    g_object_get (playbin, "flags", &flags, NULL);
    setflags();
    g_object_set (playbin, "flags", flags, NULL);

    /*
     * Set westros-sink
     */
    westerosSink = gst_element_factory_make(WESTEROS_SINK, NULL);
    fail_unless (westerosSink != NULL, "Failed to create 'westerossink' element");

    g_object_set (playbin, "video-sink", westerosSink, NULL);

    /*
     * Set the first frame recieved callback
     */
    g_signal_connect( westerosSink, "first-video-frame-callback", G_CALLBACK(firstFrameCallback), &firstFrameReceived);
    g_signal_connect( playbin, "element-setup", G_CALLBACK(elementSetupCallback), &elementsetup);
    /*
     * Set the firstFrameReceived variable as false before starting play
     */
    firstFrameReceived= false;

    /*
     * Set playbin to PLAYING
     */
    GST_FIXME( "Setting to Playing State\n");
    assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "unable to pause");
    GST_FIXME( "Set to Playing State\n");

    WaitForOperation; 

    if (AudioPTSCheckAvailable)
    {
         char audio_status[BUFFER_SIZE_SHORT] = {'\0'};
         strcat (audio_status, TDK_PATH);
         strcat (audio_status, AUDIO_DATA);
         strcat (audio_status, " &");
	     system(audio_status);
    }

    /*
     * Check if the first frame received flag is set
     */
    assert_failure (playbin,firstFrameReceived == true, "Failed to receive first video frame signal");
    data.playbin = playbin;
    getStreamProperties(&data);
    assert_failure (playbin,1 < data.n_audio,"Stream has only 1 audio stream. Audio Change requires minimum of two audio streams");
    printf("Current Audio is %d\n", data.current_audio);
    writeToFile = true;
    g_signal_emit_by_name (data.playbin, "get-audio-tags", data.current_audio, &tags);
    if (tags)
    {
        g_print ("Tags:\n");
        gst_tag_list_foreach (tags, printTag, GINT_TO_POINTER (1));
        gst_tag_list_free (tags);
    }

    for( int index=0,counter=0; counter <= data.n_audio; index++,counter++)
    {
	 /*
	  * Switch from audio stream 1 to audio stream 0 added.
	  */
         if(counter == data.n_audio)
         {
             index=0;
         }
         if( index != data.current_audio)
         {
	     if (with_pause)
	     {
		 printf("\nPausing pipeline before switching audio");
                 gst_element_set_state (data.playbin, GST_STATE_PAUSED);
		 WaitForOperation;
		 seekOperation = false;
		 PlaySeconds(playbin,2);
	     }
             printf("\nSwitching to audio stream %d\n", index);
             printf("\nSetting current-audio to %d\n",index);
             g_object_set (data.playbin, "current-audio", index, NULL);
             // Waiting for audio switch
             g_object_get (data.playbin, "current-audio", &data.current_audio, NULL);
	     sprintf(audio_switch_error_string,"FAILED : Unable to switch to %d audio stream",index);
             assert_failure (playbin,data.current_audio == index, audio_switch_error_string);
	     g_signal_emit_by_name (data.playbin, "get-audio-tags", data.current_audio, &tags);
             if (tags)
             {
                 g_print ("Tags:\n");
                 gst_tag_list_foreach (tags, printTag, GINT_TO_POINTER (1));
                 gst_tag_list_free (tags);
             }
	     if (with_pause)
             {
                 printf("\nSet pipeline to playing state\n");
                 gst_element_set_state (data.playbin, GST_STATE_PLAYING);
             }
	     assert_failure (playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &(data.currentPosition)), "Failed to query the current playback position");
             if (Flush_Pipeline)
             {
                 flushPipeline(data.playbin);
             }
             printf("\nSUCCESS : Switched to audio stream %d, Playing for %d seconds\n",index,play_timeout);
         }
	 PlaySeconds(playbin,play_timeout);
         assert_failure (playbin,audio_pts_error == false, "\nERROR : AUDIO PTS ERROR OBSERVED\n");
         assert_failure (playbin,audio_underflow_received == false, "\nERROR : AUDIO UNDERFLOW OBSERVED\n");
         if (AudioPTSCheckAvailable)
         {
             char audio_pts_status[BUFFER_SIZE_SHORT] = {'\0'};
             strcat (audio_pts_status, TDK_PATH);
             strcat (audio_pts_status, AUDIO_PTS_ERROR_FILE);
             strcat (audio_pts_status, " &");
             assert_failure (playbin,access(audio_pts_status, 0) == 0,"ERROR : AUDIO PTS ERROR OBSERVED\n");
         }
    }
    /*
     * Check for AV status if its enabled
     */
    if (true == checkAVStatus)
    {
        is_av_playing = check_for_AV_status();
        assert_failure (playbin,is_av_playing == true, "Video is not playing in TV");
    }
    GST_LOG("DETAILS: SUCCESS, Video playing successfully for all audio streams\n");


    if (playbin)
    {
       assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_NULL) !=  GST_STATE_CHANGE_FAILURE);
    }
    /*
     * Cleanup after use
     */
    fclose(filePointer);
    gst_object_unref (playbin);
}
GST_END_TEST;


/********************************************************************************************************************
 * Purpose      : Test to verify if buffer underflow signal is obtained upon audiounderrun
                  and playback is smooth after audio buffer is full again
 * Parameters   : Playback url
 ********************************************************************************************************************/
GST_START_TEST (test_audio_underflow)
{
    bool is_av_playing = false;
    GstState cur_state;
    GstElement *playbin, *westerosSink;
    bool elementsetup = false;
    gint64 seekPosition;
    gint64 currentPosition;
    bool seeked = false;
    int Seek_time_threshold = 5;
    GstStateChangeReturn state_change;

    /*
    * Create the playbin element
    */
    playbin = gst_element_factory_make(PLAYBIN_ELEMENT, NULL);
    fail_unless (playbin != NULL, "Failed to create 'playbin' element");

    /*
    * Set the url received from argument as the 'uri' for playbin
    */
    assert_failure (playbin,m_play_url != NULL, "Playback url should not be NULL");
    g_object_set (playbin, "uri", m_play_url, NULL);

    /*
    * Update the current playbin flags to enable Video and Audio Playback
    */
    g_object_get (playbin, "flags", &flags, NULL);
    setflags();
    g_object_set (playbin, "flags", flags, NULL);

    /*
    * Create westerosSink instance
    */
    westerosSink = gst_element_factory_make(WESTEROS_SINK, NULL);
    fail_unless (westerosSink != NULL, "Failed to create 'westerossink' element");
    g_object_set (playbin, "video-sink", westerosSink, NULL);


    g_signal_connect( westerosSink, "first-video-frame-callback", G_CALLBACK(firstFrameCallback), &firstFrameReceived);
    g_signal_connect( playbin, "element-setup", G_CALLBACK(elementSetupCallback), &elementsetup);


    /*
    * Set the firstFrameReceived variable as false before starting play
    */
    firstFrameReceived= false;
    audio_underflow_received = false;

    /*
     * Set playbin to PLAYING
     */
    GST_FIXME( "Setting to Playing State\n");
    assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "unable to pause");
    GST_FIXME( "Set to Playing State\n");
    WaitForOperation;

    /*
     * Check if the first frame received flag is set
     */

    assert_failure (playbin,firstFrameReceived == true, "Failed to receive first video frame signal");

    assert_failure (playbin,audioEnd != 0,"audioEnd point is not given");
    if (play_without_audio)
    {
	 bufferUnderflowTest = false;
	 ignorePlayJump = true;
         use_westerossink_fps = true;
         PlaySeconds(playbin,play_timeout + audioEnd);
    }
    else
    {
         PlaySeconds(playbin,audioEnd);
    }

    assert_failure (playbin,audio_underflow_received_global == true, "Failed to receive buffer underflow signal");
    printf ("\nDETAILS: SUCCESS, Received buffer underflow signal as expected\n");

    
    if (checkSignalTest || play_without_audio)
         goto EXIT;
	
    bufferUnderflowTest = false;

    gst_element_set_state (playbin, GST_STATE_PAUSED);
    PlaySeconds(playbin,5);
    assert_failure (playbin, gst_element_get_state (playbin, &cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS);
    assert_failure (playbin, cur_state == GST_STATE_PAUSED);
    printf("\n********Current state: %s\n",gst_element_state_get_name(cur_state));
    GST_LOG("\n********Current state: %s\n",gst_element_state_get_name(cur_state));

    /*
     * Seek to the 'audioStart' point (recieved as the input argument) to fill the audio buffer
     * And start playback
     */
    assert_failure (playbin,audioStart != 0,"audioStart point is not given");
    assert_failure (playbin,audioStart > audioEnd,"audioEnd and audioStart params are not correct");
    seekPosition = GST_SECOND * (audioStart + 2);
    seekOperation = true;
    seekSeconds = seekPosition/GST_SECOND;
    assert_failure (playbin,gst_element_seek (playbin, NORMAL_PLAYBACK_RATE, GST_FORMAT_TIME,
                                   GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, seekPosition,
                                   GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE), "Failed to seek");

    start = std::chrono::steady_clock::now();
    while(!seeked)
    {
       //Check if seek happened
       assert_failure (playbin,gst_element_query_position (playbin, GST_FORMAT_TIME, &(currentPosition)),
                                         "Failed to query the current playback position");
       //Added GST_SECOND buffer time between currentPosition and seekPosition
       if (abs( currentPosition - seekPosition) <= (GST_SECOND))
       {
           seeked = TRUE;
       }
       if (std::chrono::steady_clock::now() - start > std::chrono::seconds(Seek_time_threshold))
           break;
    }

    assert_failure (playbin,TRUE == seeked, "Seek Unsuccessfull\n");
    //Convert time to seconds
    currentPosition /= GST_SECOND;
    seekPosition /= GST_SECOND;
	printf("\nSEEK SUCCESSFULL :  CurrentPosition %lld seconds, SeekPosition %lld seconds\n", currentPosition, seekPosition);

    state_change = gst_element_get_state (playbin, &cur_state, NULL, GST_SECOND);
    assert_failure (playbin,state_change != GST_STATE_CHANGE_FAILURE, "Failed to get current playbin state");

    if ((cur_state == GST_STATE_PAUSED)  && (state_change != GST_STATE_CHANGE_ASYNC))
    {

        /*
         * Set playbin to PLAYING
         */
         GST_FIXME( "Setting to Playing State\n");
         assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "unable to pause");
         GST_FIXME( "Set to Playing State\n");
         do{
              //Waiting for state change
              state_change = gst_element_get_state (playbin, &cur_state, NULL, 10000000);
         } while (state_change == GST_STATE_CHANGE_ASYNC);

         printf("\n********Current state: %s\n",gst_element_state_get_name(cur_state));
	 assert_failure (playbin, gst_element_get_state (playbin, &cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS);
         assert_failure (playbin, cur_state == GST_STATE_PLAYING);
         GST_LOG("\n********Current state: %s\n",gst_element_state_get_name(cur_state));
    }

    PlaySeconds(playbin,play_timeout);

	printf ("\nDETAILS: SUCCESS Playback after seeking to Audio Point after buffer underflow is successfull\n");
EXIT :
    /* Check for AV status if its enabled
     */
    if (true == checkAVStatus)
    {
        is_av_playing = check_for_AV_status();
        assert_failure (playbin,is_av_playing == true, "Video is not playing in TV");
    }
    if (playbin)
    {
       gst_element_set_state(playbin, GST_STATE_NULL);
    }

    gst_object_unref (playbin);

}
GST_END_TEST;

GST_START_TEST(test_only_audio)
{
	GstElement *playbin;
	GstElement *westerosSink;
	gint flags;
	GstMessage *message;
	GstBus *bus;
	MessageHandlerData data;
	checkPTS = false;
	use_westerossink_fps = false;

	playbin = gst_element_factory_make(PLAYBIN_ELEMENT , NULL);
	fail_unless (playbin != NULL ,"Failed to create Playbin Element");

	assert_failure (playbin,m_play_url != NULL , "Playback URL should not be NULL");
	g_object_set(playbin, "uri", m_play_url, NULL);

	g_object_get(playbin,"flags",&flags,NULL);
	flags = GST_PLAY_FLAG_AUDIO;
	g_object_set(playbin,"flags", flags,NULL);


	GST_FIXME( "Setting to Playing State\n");
        assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "unable to pause");
        GST_FIXME( "Set to Playing State\n");

	PlaySeconds(playbin,play_timeout);

	if (playbin)
    	{
       		assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_NULL) !=  GST_STATE_CHANGE_FAILURE);
    	}
	
	gst_object_unref (playbin);
}
GST_END_TEST;

GST_START_TEST (test_video_bitrate)
{
    GstElement *playbin;
    GstElement *westerosSink;
    GstElement *videoSink;
    bool elementsetup = false;

        /*
    * Create the playbin element
    */
    playbin = gst_element_factory_make(PLAYBIN_ELEMENT, NULL);
    fail_unless (playbin != NULL, "Failed to create 'playbin' element");

    /*
    * Set the url received from argument as the 'uri' for playbin
    */
    assert_failure (playbin,m_play_url != NULL, "Playback url should not be NULL");
    g_object_set (playbin, "uri", m_play_url, NULL);

    /*
    * Update the current playbin flags to enable Video and Audio Playback
    */
    g_object_get (playbin, "flags", &flags, NULL);
    setflags();
    g_object_set (playbin, "flags", flags, NULL);

    /*
    * Create westerosSink instance
    */
    westerosSink = gst_element_factory_make(WESTEROS_SINK, NULL);
    fail_unless (westerosSink != NULL, "Failed to create 'westerossink' element");
    g_object_set (playbin, "video-sink", westerosSink, NULL);


    g_signal_connect( westerosSink, "first-video-frame-callback", G_CALLBACK(firstFrameCallback), &firstFrameReceived);
    g_signal_connect( playbin, "element-setup", G_CALLBACK(elementSetupCallback), &elementsetup);


    /*
    * Set the firstFrameReceived variable as false before starting play
    */
    firstFrameReceived= false;

    assert_failure (playbin,connection_speed != 0,"connection speed is not given");
    g_object_set (playbin, "connection_speed", connection_speed, NULL);
    printf("\nConnection Speed: %" G_GUINT64_FORMAT, connection_speed);
    /*
    * Set playbin to PLAYING
    */
    GST_FIXME( "Setting to Playing State\n");
    assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE);
    GST_FIXME( "Set to Playing State\n");
    WaitForOperation;

    /*
     * Check if the first frame received flag is set
     */

    assert_failure (playbin,firstFrameReceived == true, "Failed to receive first video frame signal");

    g_object_get (videoSink, "video-height", &height, NULL);
    g_object_get (videoSink, "video-width", &width, NULL);

    printf("\nVideo height = %d\nVideo width = %d", height, width);


    PlaySeconds(playbin,play_timeout);


    if (playbin)
    {
        assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_NULL) !=  GST_STATE_CHANGE_FAILURE);
    }

    gst_object_unref (playbin);
}
GST_END_TEST;


GST_START_TEST (test_color_depth)
{

    GstElement *playbin, *source, *westerosSink;
    GstElementFactory *source_factory, *sink_factory;
    bool elementsetup = false;


    playbin = gst_element_factory_make(PLAYBIN_ELEMENT, NULL);
    fail_unless (playbin != NULL, "Failed to create 'playbin' element");

    g_object_set (playbin, "video-sink",westerosSink, NULL);
    assert_failure (playbin,m_play_url != NULL, "Playback url should not be NULL");
    g_object_set (playbin, "uri", m_play_url, NULL);

    /*
     * Update the current playbin flags to enable Video and Audio Playback
     */
    g_object_get (playbin, "flags", &flags, NULL);
    setflags();
    g_object_set (playbin, "flags", flags, NULL);


    westerosSink = gst_element_factory_make(WESTEROS_SINK, NULL);
    fail_unless (westerosSink != NULL, "Failed to create 'westerossink' element");

    g_object_set (playbin, "video-sink", westerosSink, NULL);

    /*
    * Set the first frame recieved callback
    */
    g_signal_connect( westerosSink, "first-video-frame-callback", G_CALLBACK(firstFrameCallback), &firstFrameReceived);
    g_signal_connect( playbin, "element-setup", G_CALLBACK(elementSetupCallback), &elementsetup);



    /*
    * Set the firstFrameReceived variable as false before starting play
    */
    firstFrameReceived= false;


    GST_FIXME( "Setting to Playing State\n");
    assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE);
    gst_element_set_state (playbin, GST_STATE_PLAYING);
    GST_FIXME( "Set to Playing State\n");

    WaitForOperation;

    /*
    * Check if the first frame received flag is set
    */
    assert_failure (playbin,firstFrameReceived == true, "Failed to receive first video frame signal");

    PlaySeconds(playbin,play_timeout);

    string Bit_depth_1;

    if (brcm)
    {	    
     	Bit_depth_1 = executecommand (playbin,"cat /proc/brcm/video_decoder | grep Source | cut -d ' ' -f5 | tr -d ' '");
        Bit_depth_got = atoi(Bit_depth_1.c_str() );
    }
    else if (rtk)
    {
        Bit_depth_1 = executecommand (playbin,"cat /opt/TDK/colour.sh | grep LumaBitDepth | cut -d ' ' -f3 | tr -d ' '");
        Bit_depth_got = atoi(Bit_depth_1.c_str() );
    }	    
    else
    	print_pad_capabilities (westerosSink, "sink");




    assert_failure (playbin,bit_depth != 0,"bit depth is not given");

    printf("\nBit_Depth_Got = %d\n", Bit_depth_got);
    
    assert_failure (playbin,(bit_depth == Bit_depth_got), "Given bit_depth was not matched");



    if (playbin)
    {
	    assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_NULL) !=  GST_STATE_CHANGE_FAILURE);
    }

    /*
     * Cleanup after use
     */
    gst_object_unref (playbin);

}
GST_END_TEST;






static Suite *
media_pipeline_suite (void)
{
    Suite *gstPluginsSuite = suite_create ("playbin_plugin_test");
    TCase *tc_chain = tcase_create ("general");
    /*
     * Set timeout to play_timeout if play_timeout > DEFAULT_TEST_SUITE_TIMEOUT(360) seconds
     */
    guint timeout = DEFAULT_TEST_SUITE_TIMEOUT;
    if (play_timeout > DEFAULT_TEST_SUITE_TIMEOUT)
    {
        timeout = play_timeout;
    }
    tcase_set_timeout (tc_chain, timeout);
    suite_add_tcase (gstPluginsSuite, tc_chain);

    GST_INFO ("Test Case name is %s\n", tcname);
    printf ("Test Case name is %s\n", tcname);
    
    if (strcmp ("test_generic_playback", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_generic_playback);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_play_pause_pipeline", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_play_pause_pipeline);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_EOS", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_EOS);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_init_shutdown", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_init_shutdown);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_channel_change_playback", tcname) == 0)
    {
       ChannelChangeTest = true;
       tcase_add_test (tc_chain, test_generic_playback);
       GST_INFO ("tc test_channel_change_playback run successfull\n");
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_playback_latency", tcname) == 0)
    {
       latency_check_test = true;
       tcase_add_test (tc_chain, test_playback_latency);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_audio_change", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_audio_change);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_audio_change_with_pause", tcname) == 0)
    {
       with_pause = true;
       tcase_add_test (tc_chain, test_audio_change);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_frameDrop", tcname) == 0)
    {
       checkTotalFrames = true;
       tcase_add_test (tc_chain, test_frameDrop);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_playback_fps", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_frameDrop);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_rialto_playback", tcname) == 0)
    {
       use_rialto = true;
       checkPTS=false;
       tcase_add_test (tc_chain, test_generic_playback);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_rialto_play_pause", tcname) == 0)
    {
       use_rialto = true;
       checkPTS=false;
       tcase_add_test (tc_chain, test_play_pause_pipeline);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_rialto_EOS", tcname) == 0)
    {
       use_rialto = true;
       checkPTS=false;
       tcase_add_test (tc_chain, test_EOS);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_buffer_underflow_signal", tcname) == 0)
    {
       bufferUnderflowTest = true;
       tcase_add_test (tc_chain, test_buffer_underflow);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_buffer_underflow_playback", tcname) == 0)
    {
       bufferUnderflowTest = true;
       checkSignalTest = false;
       tcase_add_test (tc_chain, test_buffer_underflow);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_audio_underflow_signal", tcname) == 0)
    {
       bufferUnderflowTest = true;
       tcase_add_test (tc_chain, test_audio_underflow);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_audio_underflow_playback", tcname) == 0)
    {
       bufferUnderflowTest = true;
       checkSignalTest = false;
       tcase_add_test (tc_chain, test_audio_underflow);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_resolution", tcname) == 0)
    {
       ResolutionTest = true;
       tcase_add_test (tc_chain, test_generic_playback);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_rialto_resolution", tcname) == 0)
    {
       ResolutionTest = true;
       use_rialto = true;
       tcase_add_test (tc_chain, test_generic_playback);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_resolution_up", tcname) == 0)
    {
       ResolutionSwitchTest = true;
       tcase_add_test (tc_chain, test_generic_playback);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_resolution_down", tcname) == 0)
    {
       ResolutionSwitchTest = true;
       resolution_test_up = false;
       tcase_add_test (tc_chain, test_generic_playback);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_audio_sampling_rate", tcname) == 0)
    {
       checkAudioSamplingrate = true;
       tcase_add_test (tc_chain, test_play_pause_pipeline);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_only_audio", tcname) == 0)
    {
       checkAudioSamplingrate = true;
       tcase_add_test (tc_chain, test_only_audio);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_video_bitrate", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_video_bitrate);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_color_depth", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_color_depth);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else
    {
       printf("\nNo such testcase is present in app");
       GST_INFO ("FAILURE\n");
    }
    return gstPluginsSuite;
}



int main (int argc, char **argv)
{
    Suite *testSuite;
    int returnValue = 0;
    int arg = 0;
    char *timeoutparser;
    std::vector<int> TimeoutList;
    char* timeout = NULL;

    /*
     * Get TDK path
     */
    if (getenv ("TDK_PATH") != NULL)
    {
    	strcpy (TDK_PATH, getenv ("TDK_PATH"));
    }
    else
    {
    	GST_ERROR ("Environment variable TDK_PATH should be set!!!!");
    	printf ("Environment variable TDK_PATH should be set!!!!");
	returnValue = 0;
	goto exit;
    }
    if (getenv ("AUDIO_PTS_CHECK") != NULL)
    {
        AudioPTSCheckAvailable = true;
    }
    if (argc == 2)
    {
    	strcpy (tcname, argv[1]);

    	GST_INFO ("\nArg 2: TestCase Name: %s \n", tcname);
    	printf ("\nArg 2: TestCase Name: %s \n", tcname);
    }
    else if (argc >= 3)
    {
        strcpy (tcname, argv[1]);
	if ((strcmp ("test_init_shutdown", tcname) == 0) || 
	    (strcmp ("test_play_pause_pipeline", tcname) == 0) || 
            (strcmp ("test_generic_playback", tcname) == 0) ||
            (strcmp ("test_playback_latency", tcname) == 0) ||
            (strcmp ("test_audio_change", tcname) == 0) ||
            (strcmp ("test_audio_change_with_pause", tcname) == 0) ||
	    (strcmp ("test_frameDrop", tcname) == 0) ||
	    (strcmp ("test_buffer_underflow_signal", tcname) == 0) ||
	    (strcmp ("test_buffer_underflow_playback", tcname) == 0) ||
	    (strcmp ("test_audio_underflow_signal", tcname) == 0) ||
	    (strcmp ("test_audio_underflow_playback", tcname) == 0) ||
	    (strcmp ("test_resolution", tcname) == 0) ||
	    (strcmp ("test_resolution_down", tcname) == 0) ||
	    (strcmp ("test_resolution_up", tcname) == 0) ||
	    (strcmp ("test_playback_fps", tcname) == 0) ||
	    (strcmp ("test_rialto_playback", tcname) == 0) ||
	    (strcmp ("test_rialto_play_pause", tcname) == 0) ||
	    (strcmp ("test_rialto_EOS", tcname) == 0) ||
	    (strcmp ("test_rialto_resolution", tcname) == 0) ||
            (strcmp ("test_EOS", tcname) == 0) ||
	    (strcmp ("test_audio_sampling_rate", tcname) == 0) ||
	    (strcmp ("test_only_audio", tcname) == 0) ||
	    (strcmp ("test_video_bitrate", tcname) == 0) ||
	    (strcmp ("test_color_depth", tcname) == 0))
	{
	    strcpy(m_play_url,argv[2]);
            arg = 3;

            for (; arg < argc; arg++)
            {
                if (strcmp ("checkavstatus=yes", argv[arg]) == 0)
                {
                    checkAVStatus = true;
                }
                if (strstr (argv[arg], "timeout=") != NULL)
                {
                    strtok (argv[arg], "=");
      		    play_timeout = atoi (strtok (NULL, "="));
                }
		if (strstr (argv[arg], "videoEnd=") != NULL)
                {
                    strtok (argv[arg], "=");
                    videoEnd = atoi (strtok (NULL, "="));
                }
		if (strstr (argv[arg], "videoStart=") != NULL)
                {
                    strtok (argv[arg], "=");
                    videoStart = atoi (strtok (NULL, "="));
                }
		if (strstr (argv[arg], "audioEnd=") != NULL)
                {
                    strtok (argv[arg], "=");
                    audioEnd = atoi (strtok (NULL, "="));
                }
		if (strstr (argv[arg], "audioStart=") != NULL)
                {
                    strtok (argv[arg], "=");
                    audioStart = atoi (strtok (NULL, "="));
                }
		if (strstr (argv[arg], "connection_speed=") != NULL)
                {
                    strtok (argv[arg], "=");
                    connection_speed = atoi (strtok (NULL, "="));
                }
		if (strstr (argv[arg], "height=") != NULL)
                {
                    strtok (argv[arg], "=");
                    height = atoi (strtok (NULL, "="));
                }
		if (strstr (argv[arg], "bit_depth=") != NULL)
                {
                    strtok (argv[arg], "=");
                    bit_depth = atoi (strtok (NULL, "="));
                }
		if (strcmp ("checkAudioFPS=no", argv[arg]) == 0)
                {
                    use_audioSink = false;
                }
		if (strcmp ("checkASR=yes", argv[arg]) == 0)
                {
                    checkAudioSamplingrate = true;
                }
		if (strcmp ("checkPTS=no", argv[arg]) == 0)
                {
		    checkPTS = false;
                }
		if (strcmp ("justPrintPTS", argv[arg]) == 0)
		{
		      justPrintPTS = true;
		}
		if (strstr (argv[arg], "checkWesterosFPS=yes") != NULL)
                {
                    use_westerossink_fps = true;
                }
		if (strcmp ("checkFPS=no", argv[arg]) == 0)
	        {
		    use_westerossink_fps = false;
		}
		if (strcmp ("playWithoutVideo", argv[arg]) == 0)
		{
                    use_westerossink_fps = false;		    
		    play_without_video = true;
		}
		if (strcmp ("playWithoutAudio", argv[arg]) == 0)
                {
                    play_without_audio = true;
                }
		if (strcmp ("useProcForFPS", argv[arg]) == 0)
		{
		    useProcForFPS = true;
                }
		if (strstr (argv[arg], "checkResolution=") != NULL)
                {
                    strtok (argv[arg], "=");
                    resolution = (strtok (NULL, "="));
                }
		if (strstr (argv[arg], "fps=") != NULL)
                {
                    strtok (argv[arg], "=");
                    fps = atoi (strtok (NULL, "="));
                }
		if (strcmp ("ignorePlayJump", argv[arg]) == 0)
         	{
            	    ignorePlayJump = true;
         	}
		if (strcmp ("buffering_flag=no", argv[arg]) == 0)
                {
                    buffering_flag = false;
                }
		if (strstr (argv[arg], "totalFrames=") != NULL)
                {
                    strtok (argv[arg], "=");
                    totalFrames = atoi (strtok (NULL, "="));
                }
		if (strstr (argv[arg], "4kNotSupported") != NULL)
		{
		    UHD_Not_Supported = true;
		}
                if (strstr (argv[arg], "withoutFlush") != NULL)
                {
                    Flush_Pipeline = false;
                }
		if (strstr (argv[arg], "sampling_rate=") != NULL)
		{
		    strtok (argv[arg], "=");
		    sampling_rate = atoi (strtok (NULL, "="));
		}
            }

            printf ("\nArg : TestCase Name: %s \n", tcname);
            printf ("\nArg : Playback url: %s \n", m_play_url);

        }
        else if (strcmp ("test_channel_change_playback", tcname) == 0)
        {
            strcpy(m_play_url,argv[2]);
            strcpy(channel_url,argv[3]);

            for (arg = 4; arg < argc; arg++)
            {
                if (strcmp ("checkavstatus=yes", argv[arg]) == 0)
                {
                    checkAVStatus = true;
                }
		if (strcmp ("checkAudioFPS=no", argv[arg]) == 0)
                {
                    use_audioSink = false;
		}
		if (strcmp ("checkPTS=no", argv[arg]) == 0)
		{
		    checkPTS = false;
		}
		if (strcmp ("checkFPS=no", argv[arg]) == 0)
                {
                    use_westerossink_fps = false;
                }
                if (strstr (argv[arg], "timeout=") != NULL)
                {
                    strtok (argv[arg], "=");
                    timeoutparser = strtok(NULL, "=");
                    timeout = strtok (timeoutparser, ",");
                    while (timeout != NULL)
                    {
                        TimeoutList.push_back(atoi(timeout));
                        timeout = strtok (NULL, ",");
                    }
                    SecondChannelTimeout  = TimeoutList[1];
                    play_timeout = TimeoutList[0];
                }
            }
            printf ("\nArg : TestCase Name: %s \n", tcname);
            printf ("\nArg : First Channel: %s \n", m_play_url);
            printf ("\nArg : FirstChannel timeout: %d \n", play_timeout);
            printf ("\nArg : Second Channel : %s \n",channel_url);
            printf ("\nArg : SecondChannel timeout: %d\n", SecondChannelTimeout);
        }
    }
    else
    {
        printf ("FALIURE : Insufficient arguments\n");
        returnValue = 0;
	goto exit;
    }
    gst_check_init (&argc, &argv);
    testSuite = media_pipeline_suite ();
    returnValue =  gst_check_run_suite (testSuite, "playbin_plugin_test", __FILE__);
exit:
    return returnValue;
}


