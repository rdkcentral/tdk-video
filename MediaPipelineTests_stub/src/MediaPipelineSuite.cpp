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
#include <ctime>
#include <curl/curl.h>
#include <sstream>
#include <chrono>
#include <bits/stdc++.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <gst/audio/audio.h>

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
#define MilliSleep(RunSeconds)          start = std::chrono::steady_clock::now(); \
                                        Runforseconds = RunSeconds; \
                                        while(1) { \
                                        if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(Runforseconds)) \
                                             break; \
                                        }
#define WaitForOperation                Sleep(5)
#define AUDIO_DATA                      "/CheckAudioStatus.sh getPTS "
#define AUDIO_PTS_ERROR_FILE            "/audio_pts_error"
#define RESOLUTION_OFFSET               5
#define BUFF_LENGTH 			512
#define ENV_FILE                        "/opt/TDK.env"
#define PROGRESS_BAR_WIDTH              50
#define CHUNK_SIZE                      4096
#define PROGRESS_BAR_DISPLAY_INTERVAL   5 //seconds 

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
gdouble volume_set = 1000;
bool volume_stress = false;
bool avsync_enabled = false;
bool forward_events = true;
string audiosink;
bool ignoreError = false;
bool checkNewPlay = false;
bool only_audio = false;
bool only_video = false;
bool checkEachSecondPlayback = false;
bool use_appsrc = false;
int ReadSize = 0;
guint64 total_bytes_fed = 0;
guint64 BYTES_THRESHOLD = 0;
float previous_progress_percentage = 0.0;
bool force_appsrc = false;
bool finish_feed = false;
bool video_underflow_test = false;
bool audio_underflow_test = false;
bool forceAudioSink = false;

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
 *
 */

typedef struct SinkData {
    GstElement *sink;
    GstStructure *structure;
    guint64 rendered_frames;
    guint64 previous_rendered_frames;
    guint64 dropped_frames;
    gint frame_buffer;
    gint height;
    gint width;
    gint64 pts;
    gint64 old_pts;
    gint pts_buffer;
    gboolean UnderflowReceived;
} SinkElementData;

typedef struct _App App;

struct _App
{
  GstElement *appsrc;
  guint sourceid;
  GMappedFile *file;
  gchar *data;
  gsize length;
  guint64 offset;
};

App s_app;

typedef struct CustomData {
    GstElement *playbin;                /* Playbin element handle */
    App app;
    SinkElementData westerosSink;       /* westerossink element handle */
    SinkElementData audioSink;          /* audioSink element handle */
    gboolean pipelineInitiation;        /* Variable to indicate whether pipeline playback validation is just started */
    GstState cur_state;                 /* Variable to store the current state of pipeline */
    gint64 seekPosition;                /* Variable to store the position to be seeked */
    gdouble seekSeconds;                /* Variable to store the position to be seeked in seconds */
    gint64 currentPosition;             /* Variable to store the current position of pipeline */
    gint64 previousposition;            /* Variable to store the previous position of pipeline */
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
   gint64 old_pts = 0;
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
   guint64 previous_rendered_frames = 0;
   //gint queued_frames;
   guint64 dropped_frames;
   guint64 rendered_frames;
   guint64 dropped_frames_audio;
   guint64 rendered_frames_audio;
   guint64 previous_rendered_frames_audio;
   guint64 audio_sampling_rate;
   int audio_sampling_rate_buffer = 3;
   int audio_sampling_diff = 0;
   int rate_diff_threshold = -3;
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

   if (only_audio)
   {
        checkPTS=false;
        use_westerossink_fps=false;
   }

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

   if (use_audioSink)
   {	   
       g_object_get (playbin,"audio-sink",&audioSink,NULL);
       string audiosink_name;
       g_object_get (audioSink,"name",&audiosink_name,NULL);
       printf("\nAudioSink used for this pipeline is %s\n",audiosink_name.c_str());
   }

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


		audio_sampling_diff = (int)audio_sampling_rate - sampling_rate;
		printf(" Sampling rate = %d",sampling_rate);
		printf(" Audio_sampling_rate_diff = %d",audio_sampling_diff);


		if (audio_sampling_diff < 0)
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
	if (data.eosDetected == TRUE)
             break;

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
	   assert_failure (playbin,frame_rate != 0 , "Rendered frames equals to previous rendered frames, Video is not playing");
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
                     rate_diff_threshold = -5;
   	}
   }

   printf("\nExiting from PlaySeconds, currentPosition is %0.2f\n",(_currentPosition/GST_SECOND));
   gst_object_unref (bus);
}


void PlaybackValidation(MessageHandlerData *data, int seconds)
{
    if (only_audio)
    {
	 checkPTS=false;
	 use_westerossink_fps=false;
    }
  
    //thresholds for pts and frames valdiation	
    if (play_without_audio || play_without_video)
    {
	 data->westerosSink.frame_buffer = 5;
	 data->audioSink.frame_buffer = 5;
    }
    else
    {	    
         data->westerosSink.frame_buffer = 3;
         data->audioSink.frame_buffer = 3;
    }
    data->westerosSink.pts_buffer = 20;
    int audio_sampling_rate_buffer = 3;
    int rate_diff_threshold = -3;
    GstMessage *message;

    // Resolution Switch Test Initializations
    vector<int> resList;
    int resItr = 0;
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

    // Get some parameters initially from pipeline
    if (data->pipelineInitiation)
    {
        assert_failure (data->playbin,gst_element_query_position (data->playbin, GST_FORMAT_TIME, &(data->previousposition)), "Failed to query the current playback position");
	data->pipelineInitiation = false;
    }

    if (use_audioSink)
    {
        g_object_get (data->playbin,"audio-sink",&(data->audioSink.sink),NULL);
        string audiosink_name;
        g_object_get (data->audioSink.sink,"name",&audiosink_name,NULL);
        printf("\nAudioSink used for this pipeline is %s\n",audiosink_name.c_str());
    }

    // Get pipeline state
    assert_failure (data->playbin, gst_element_get_state (data->playbin, &(data->cur_state), NULL, 0) == GST_STATE_CHANGE_SUCCESS);
    GstBus *bus;
    bus = gst_element_get_bus (data->playbin);

    // seconds paramters for loop monitoring
    int milliSeconds = 0;
    int previous_seconds = 0;
    int second_count = 0;

    // loop timer
    auto loopStart = std::chrono::high_resolution_clock::now();

    while(1)
    {
	 /* Loop break condition   
	  * Break after executing the desired number of seconds
	  */
         if (std::chrono::high_resolution_clock::now() - loopStart > std::chrono::seconds(seconds))
              break;
     
	 // Second Counter
	 second_count = milliSeconds/1000;

	 message = gst_bus_pop_filtered(bus,(GstMessageType)((GstMessageType) GST_MESSAGE_ERROR | (GstMessageType) GST_MESSAGE_EOS ));
         if (NULL != message)
         {
             handleMessage (data, message);
         }

	 if (data->eosDetected == TRUE)
	     break;


	 /* Playback Position Validation
	  * By default Playback Position Validation is for each 100 milliseconds
	  * If user wants to override and make it for each second once checkEachSecondPlayback must be set to true
	  */
	 if (((checkEachSecondPlayback) && ((second_count > previous_seconds) || (milliSeconds == 0))) || (!checkEachSecondPlayback))
         {
       
              assert_failure (data->playbin,gst_element_query_position (data->playbin, GST_FORMAT_TIME, &(data->currentPosition)), "Failed to query the current playback position");
              auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
              std::tm* timeStruct = std::localtime(&currentTime);
              printf("\nTDK LOG :  %02d:%02d:%02d  -- ", timeStruct->tm_hour, timeStruct->tm_min, timeStruct->tm_sec);
              //printf(" Playback position: %" GST_TIME_FORMAT "", GST_TIME_ARGS(data->currentPosition));
	      printf(" Playback position: %" G_GINT64_FORMAT ".%03" G_GINT64_FORMAT "",  GST_TIME_AS_SECONDS(data->currentPosition), GST_TIME_AS_MSECONDS(data->currentPosition) % 1000);

              //printf(" Expected Playback position:  %" GST_TIME_FORMAT "", GST_TIME_ARGS(data->previousposition));
	      printf(" Expected Playback position:  %" G_GINT64_FORMAT ".%03" G_GINT64_FORMAT "",  GST_TIME_AS_SECONDS(data->previousposition), GST_TIME_AS_MSECONDS(data->previousposition) % 1000);
              gint64 position_diff = data->currentPosition - data->previousposition;
              gdouble position_diff_seconds = static_cast<gdouble>(position_diff) / GST_SECOND;

              printf("   Position diff: %.5f seconds", abs(position_diff_seconds));
              if(!ignorePlayJump)
              {
                  if (abs(position_diff_seconds) > 0.9)
                       gst_element_set_state (data->playbin, GST_STATE_NULL);
                  fail_unless( position_diff_seconds < 0.9,"Difference in expected position is large \nTDK LOG :  %02d:%02d:%02d  -- Playback position: %" GST_TIME_FORMAT " Expected Playback position:  %" GST_TIME_FORMAT "    Position diff: %.5f seconds",timeStruct->tm_hour, timeStruct->tm_min, timeStruct->tm_sec,GST_TIME_ARGS(data->currentPosition),GST_TIME_ARGS(data->previousposition),abs(position_diff_seconds));
              }
	      if (data->cur_state == GST_STATE_PLAYING)
              {
                  data->previousposition += 100 * GST_MSECOND;

              }
	 }

         /* Video PTS validation - obtained from westerossink
          * Done for 100 milliseconds
          * If needed for each second validation instead of 100 milliseconds, use the below format in if condition
          * if ((checkPTS) && (second_count > previous_seconds))
          */
	 if ((((checkEachSecondPlayback) && (second_count > previous_seconds)) || (!checkEachSecondPlayback)) && (checkPTS))
         {
             g_object_get (data->westerosSink.sink,"video-pts",&(data->westerosSink.pts),NULL);
             printf("  PTS : %lld ",data->westerosSink.pts);
             if (((data->westerosSink.pts ==0) || (data->westerosSink.old_pts >= data->westerosSink.pts)) &&  (data->cur_state == GST_STATE_PLAYING))
             {
                 data->westerosSink.pts_buffer -= 1;
                 printf("\nWARNING : Video not playing");
             }
	     if (((data->westerosSink.pts ==0) || (data->westerosSink.old_pts != data->westerosSink.pts)) &&  (data->cur_state == GST_STATE_PAUSED))
	     {
                 data->westerosSink.pts_buffer -= 1;
                 printf("\nWARNING : Video not paused");
             }
             if (!justPrintPTS)
             {
                 assert_failure (data->playbin,data->westerosSink.pts_buffer != 0 , "Video is not playing according to video-data->westerosSink.pts check of westerosSink");
             }
             data->westerosSink.old_pts = data->westerosSink.pts;
         }

	 /* Video Frames validation along with seconds counter
	  * Video validation is done for each second instead of 100 milliseconds
	  */
	 if ((use_westerossink_fps) && (second_count > previous_seconds))
         {
             g_object_get (data->westerosSink.sink,"stats",&(data->westerosSink.structure),NULL);
	     gst_structure_get_uint64(data->westerosSink.structure, "rendered", &(data->westerosSink.rendered_frames));
	     if (data->westerosSink.structure && (gst_structure_has_field(data->westerosSink.structure, "dropped") || gst_structure_has_field(data->westerosSink.structure, "rendered")))
	     {
		 gst_structure_get_uint64(data->westerosSink.structure, "dropped", &(data->westerosSink.dropped_frames));
                 gst_structure_get_uint64(data->westerosSink.structure, "rendered", &(data->westerosSink.rendered_frames));
		 printf("\n\nVideo Frames :");
                 printf(" Dropped: %" G_GUINT64_FORMAT, data->westerosSink.dropped_frames);
		 printf(" Rendered: %" G_GUINT64_FORMAT , data->westerosSink.rendered_frames);

		 assert_failure (data->playbin,data->westerosSink.rendered_frames != 0 , "Video rendered_frames is coming as 0");
		 if (((data->westerosSink.rendered_frames <= data->westerosSink.previous_rendered_frames)) &&  (data->cur_state == GST_STATE_PLAYING))
                    data->westerosSink.frame_buffer -= 1;
		 else if (((data->westerosSink.rendered_frames != data->westerosSink.previous_rendered_frames)) &&  (data->cur_state == GST_STATE_PAUSED))
		    data->westerosSink.frame_buffer -= 1;

		 assert_failure (data->playbin,data->westerosSink.frame_buffer != 0 , "Video frames are not rendered properly");
		 data->westerosSink.previous_rendered_frames = data->westerosSink.rendered_frames;
	     }

	 }

	 /* Audio Frames validation along with seconds counter
	  * Audio validation is done for each second instead of 100 milliseconds
	  */
         if ((use_audioSink) && (second_count > previous_seconds))
         {
             g_object_get (data->audioSink.sink,"stats",&(data->audioSink.structure),NULL);
             gst_structure_get_uint64(data->audioSink.structure, "rendered", &(data->audioSink.rendered_frames));
             if (data->audioSink.structure && (gst_structure_has_field(data->audioSink.structure, "dropped") || gst_structure_has_field(data->audioSink.structure, "rendered")))
             {
                 gst_structure_get_uint64(data->audioSink.structure, "dropped", &(data->audioSink.dropped_frames));
                 gst_structure_get_uint64(data->audioSink.structure, "rendered", &(data->audioSink.rendered_frames));
		 if (!use_westerossink_fps)
		     printf("\n");
                 printf("\nAudio Frames :");
                 printf(" Dropped: %" G_GUINT64_FORMAT, data->audioSink.dropped_frames);
                 printf(" Rendered: %" G_GUINT64_FORMAT "\n", data->audioSink.rendered_frames);

		 // For play_without_audio test , after audioEnd point stop verifying audio frames
		 if ((play_without_audio) && (data->currentPosition/GST_SECOND >= audioEnd))
		     use_audioSink = false;

                 if ((data->audioSink.rendered_frames <= data->audioSink.previous_rendered_frames) &&  (data->cur_state == GST_STATE_PLAYING))
                    data->audioSink.frame_buffer -= 1;
		 else if ((data->audioSink.rendered_frames != data->audioSink.previous_rendered_frames) &&  (data->cur_state == GST_STATE_PAUSED))
		    data->audioSink.frame_buffer -= 1;

                 assert_failure (data->playbin,data->audioSink.frame_buffer != 0 , "Audio frames are not rendered properly");
             }

	     if ((checkAudioSamplingrate) && (data->cur_state == GST_STATE_PLAYING))
	     {
		 int audio_sampling_rate =  data->audioSink.rendered_frames -  data->audioSink.previous_rendered_frames;
                 printf("\nAudio sampling rate = %d", audio_sampling_rate);
                 int audio_sampling_diff = (int)audio_sampling_rate - sampling_rate;
		 printf(" Sampling rate = %d",sampling_rate);
		 printf(" Audio_sampling_rate_diff = %d",audio_sampling_diff);
		 if (audio_sampling_diff < 0)
		 {
			if (audio_sampling_diff <= rate_diff_threshold)
				audio_sampling_rate_buffer -= 1;
		 } 
		 assert_failure (data->playbin,audio_sampling_rate_buffer != 0, "Audio sampling rate was not rendered properly");

		 if (sampling_rate == 96)
                     rate_diff_threshold = -5;
	     }
	     data->audioSink.previous_rendered_frames =  data->audioSink.rendered_frames;

         }

	 // Video underflow break condition
	 if(videoUnderflowReceived && bufferUnderflowTest)
         {
             printf("\nVideo Underflow received breaking from PlaybackValidation");
             break;
         }

	 // Audio underflow break condition
	 if(audio_underflow_received_global && bufferUnderflowTest)
         {
             printf("\nAudio Underflow received breaking from PlaybackValidation");
             break;
         }

	 // Resolution Switch Test
	 if (ResolutionSwitchTest)
	 {
             int new_height, new_width;
	     g_object_get (data->westerosSink.sink, "video-height", &new_height, NULL);
             g_object_get (data->westerosSink.sink, "video-width", &new_width, NULL);
	     if (abs(data->westerosSink.height - resList[resItr]) < RESOLUTION_OFFSET)
	     {
	         ResolutionCount++;
		 resItr++;
             }
             if ( (new_height != data->westerosSink.height) || (new_width != data->westerosSink.width))
             {
                 data->westerosSink.height = new_height;
                 data->westerosSink.width = new_width;
                 printf("\nVideo height = %d\nVideo width = %d", data->westerosSink.height, data->westerosSink.width);
	         printf("\nres-comparison value = %d",resList[resItr]);
             }
	     if(ResolutionCount == TOTAL_RESOLUTIONS_COUNT)
             {
	         printf("\nPipeline played all resolutions\n");
                 break;
	     }
	 }

         // Seconds Counter
	 if (second_count > previous_seconds)
	     previous_seconds += 1;

         // Sleeping for  100 milliseconds
	 MilliSleep(100);
	 milliSeconds += 100;
     }

     printf("\nExiting from PlaybackValidation, currentPosition is %lld\n",(data->currentPosition)/GST_SECOND);
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
#ifdef NATIVE_VIDEO
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

bool directoryExists(const std::string& path) 
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false;
    else if (info.st_mode & S_IFDIR)
        return true;
    else
        return false;
}

static string getAudioSink()
{
   FILE *fp = NULL;
   char readRespBuff[10] = { '\0' };
   printf ("\nObtaining the SOC of the DUT\n");
   fp = popen("cat /etc/device.properties | grep SOC | cut -d '=' -f2 ","r");
   if(fp == NULL)
   {
       printf("\npopen failed\n");
       return "";
   }
   while(fgets(readRespBuff,sizeof(readRespBuff),fp) != NULL);
   readRespBuff[strcspn(readRespBuff, "\n")] = '\0';
   pclose(fp);
   printf ("SOC obtained from DUT : %s\n",readRespBuff);
   if (strcmp(readRespBuff,"AMLOGIC") == 0)
       return "amlhalasink";
   else if (strcmp(readRespBuff,"RTK") == 0)
       return "rtkaudiosink";
   else if (directoryExists("/proc/brcm"))
       return "brcmaudiosink";
   else
       printf("\nPlatform audio-sink is not configured for this DUT\n");
   return "";
}



/******************************************************************************************************************
 * Purpose:                Callback function to initialize appsrc with corresponding callbacks
 *****************************************************************************************************************/
static void found_source(GstElement* playbin, GstElement* source, gpointer data)
{
   MessageHandlerData *newData = (MessageHandlerData *)data;
   printf("\nGot source setup callback\n");
   g_object_get(playbin,"source",&(newData->app.appsrc), NULL);
   g_object_set(newData->app.appsrc, "size", (gint64)newData->app.length, NULL);
}

/******************************************************************************************************************
 * Purpose:                Callback function to capture underflow signal from audiosink
 *****************************************************************************************************************/
static void AudioUndeflowCallback(GstElement *sink, guint size, void *context, gpointer data)
{
   bool *gotVideoUnderflow = (bool*)data;
   if (!audio_underflow_received)
   printf ("\nERROR : Received audio buffer underflow signal\n");
   audio_underflow_received_global = true;
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
static void elementSetupCallback (GstElement* playbin, GstElement* element, MessageHandlerData  *data)
{
    if (g_str_has_prefix(GST_ELEMENT_NAME(element), "appsrc")) {
	printf ("\nGot appsrc\n");
	data->app.appsrc = element;
    }
    if (g_str_has_prefix(GST_ELEMENT_NAME(element), "brcmaudiodecoder")) {
        printf("\nGot brcmaudiodecoder\n");
	brcm = true;
        g_object_set(G_OBJECT(element), "enable-svp", true , NULL);
        g_signal_connect( element, "buffer-underflow-callback", G_CALLBACK(AudioUndeflowCallback), &audio_underflow_received);
        g_signal_connect( element, "first-audio-frame-callback", G_CALLBACK(FirstFrameAudioCallback), &audio_started);
        g_signal_connect( element, "pts-error-callback", G_CALLBACK(AudioPTSErrorCallback), &audio_pts_error);
    }
    if (g_str_has_prefix(GST_ELEMENT_NAME(element), "brcmaudiosink")) {
	printf ("\nGot brcmaudiosink\n");
    }
    if (g_str_has_prefix(GST_ELEMENT_NAME(element), "amlhalasink")) {
        printf("\nGot amlhalasink\n");
        g_signal_connect( element, "underrun-callback", G_CALLBACK(AudioUndeflowCallback), &audio_underflow_received);
    }
    if (g_str_has_prefix(GST_ELEMENT_NAME(element), "rtkaudiosink")) {
        printf("\nGot rtkaudiosink\n");
	rtk = true;
        g_signal_connect( element, "buffer-underflow-callback", G_CALLBACK(AudioUndeflowCallback), &audio_underflow_received);
    }

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

/******************************************************************************************************************
 * Purpose:                String compare: check whether the string ends with the corresponding suffix
 *****************************************************************************************************************/
bool suffixCompare(const char *str, const char *suffix) {
    size_t strLength = strlen(str);
    size_t suffixLength = strlen(suffix);

    if (strLength < suffixLength) {
        return false;
    }

    const char *found = strstr(str + (strLength - suffixLength), suffix);
    return found != nullptr && found == str + (strLength - suffixLength); // Check if found at the end of the string
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
Purpose:               To obtain the file size
*********************************************************************************************************************/
long GetFileSize(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

/********************************************************************************************************************
Purpose:               To flush the pipeline using seek
*********************************************************************************************************************/
void flushPipeline(MessageHandlerData *data)
{
    seekOperation = true;
    printf("\nFlushing Pipeline after switch\n");
    assert_failure (data->playbin,gst_element_query_position (data->playbin, GST_FORMAT_TIME, &(data->currentPosition)),
                                         "Failed to query the current playback position");
    seekSeconds = (data->currentPosition)/GST_SECOND;
    assert_failure (data->playbin,gst_element_seek (data->playbin, NORMAL_PLAYBACK_RATE, GST_FORMAT_TIME,
                                   GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, data->currentPosition,
                                   GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE), "Failed to seek");
    Sleep(2);
    audio_underflow_received = false;
    assert_failure (data->playbin,gst_element_query_position (data->playbin, GST_FORMAT_TIME, &(data->previousposition)), "Failed to query the current playback position");
    return;
}

/********************************************************************************************************************
Purpose:               Callback function to set a variable to true on receiving first frame
*********************************************************************************************************************/
static void firstFrameCallback(GstElement *westerosSink, guint size, void *context, gpointer data)
{
   bool *gotFirstFrameSignal = (bool*)data;

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
message [IN]          - GstMessage* handle to the message received from bus
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
  {
      printf("\nValue got is %s\n",string);
      Bit_depth_got = atoi(string);
  }

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


static void print_pad_capabilities (GstElement *element, const gchar *pad_name)
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
	Purpose: To set and get the audio volume
*********************************************************************************************************************/

static void play_set_relative_volume (GstElement *playbin)
{
  gdouble get_volume, set_volume, volume;

  volume = gst_stream_volume_get_volume (GST_STREAM_VOLUME (playbin),
      GST_STREAM_VOLUME_FORMAT_CUBIC);

  printf ("Previous volume : %.0f%% \n", volume * 100);

  printf  ("Setting volume to %f \n", volume_set);
  
  set_volume = volume_set;

  printf ("Volume setted : %.0f%% \n", set_volume * 100);

  gst_stream_volume_set_volume (GST_STREAM_VOLUME (playbin),
      GST_STREAM_VOLUME_FORMAT_CUBIC, set_volume);

  get_volume = gst_stream_volume_get_volume (GST_STREAM_VOLUME (playbin),
      GST_STREAM_VOLUME_FORMAT_CUBIC);
 
  printf ("Volume: %.0f%% \n", get_volume * 100);

  assert_failure (playbin,set_volume == get_volume,"Failed to set the given volume");
}

/*************************************************** Appsrc reading start *******************************************/

/********************************************************************************************************************
Purpose:               Read from file and save to Data
*********************************************************************************************************************/
int readFromFile(MessageHandlerData *data, const char *ReadFile, size_t maxBytes)
{
    FILE *file;
    gchar *Data;
    size_t bytesRead;
    size_t totalBytesRead = 0;
    const size_t chunkSize = 4096;

    file = fopen(ReadFile, "rb");
    fail_unless(file != NULL, "failed to open file: %s\n", strerror(errno));

    printf("\nReading from file %s", ReadFile);

    // Allocate initial memory
    Data = (gchar *)malloc(chunkSize);
    if (Data == NULL) {
        fclose(file);
	fail_unless(Data != NULL, "failed to allocate memory\n");
    }

    // Read in chunks
    while (1)
    {
	// if maxbytes is not given , read full file
	if ((maxBytes > 0) && (totalBytesRead < maxBytes))
	    break;
	
        bytesRead = fread(Data + totalBytesRead, 1, chunkSize, file);
        if (bytesRead <= 0) {
            break; // Exit the loop if no more data to read
        }

        totalBytesRead += bytesRead;

        // Resize the buffer if necessary
        Data = (gchar *)realloc(Data, totalBytesRead + chunkSize);
        if (Data == NULL) {
            fclose(file);
	    fail_unless(Data != NULL, "failed to reallocate memory\n");
        }
    }

    fclose(file);

    // Trim the buffer to the actual amount read (up to maxBytes)
    Data = (gchar *)realloc(Data, totalBytesRead);
    fail_unless(Data != NULL, "failed to trim buffer\n");

    data->app.data = Data;
    data->app.length = totalBytesRead;

    return 0;
}

/********************************************************************************************************************
Purpose:               Read from curl and save to Data
*********************************************************************************************************************/

// Memory structure for curl
struct MemoryStruct {
  char *memory;
  size_t size;
};

struct MemoryStruct chunk;

/*
 * Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
 * Licensed under the curl License
 */
// Write memory callback used to get the data from the url
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr =  (char *)realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

// Displaying progress bar while download of data
auto lastDisplayTime = std::chrono::time_point<std::chrono::steady_clock>::min();
void displayProgressBar(const char* Process, float progress_percentage)
{
  int loop_iterator;
  int num_blocks = (int)(progress_percentage / (100.0 / PROGRESS_BAR_WIDTH));
  auto currentTime = std::chrono::steady_clock::now();
  auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastDisplayTime).count();
  //if ((abs(progress_percentage - previous_progress_percentage) > 10) || progress_percentage == 100.0)
  if ((elapsedTime >= PROGRESS_BAR_DISPLAY_INTERVAL) || progress_percentage == 100.0 ||(lastDisplayTime == std::chrono::time_point<std::chrono::steady_clock>::min()))
  {
        printf("\r%s Progress: [",Process);
        for (loop_iterator = 0; loop_iterator < num_blocks; ++loop_iterator)
        {
           printf("*");
        }
        for (; loop_iterator < PROGRESS_BAR_WIDTH; ++loop_iterator)
        {
           printf(" ");
        }
        printf("] %.2f%% \n", progress_percentage);
        fflush(stdout);
        previous_progress_percentage = progress_percentage;
        lastDisplayTime = std::chrono::steady_clock::now();
  }
}

// Progress of curl download
long filesize;
static int ProgressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{

  float progress_percentage = ((float)chunk.size / filesize) * 100;
  displayProgressBar("Download",progress_percentage);
  return 0;
}

//Struct to define curl Header Data
struct HeaderData {
    long contentLength;
};

//callback is called to process curl header data
size_t parseHeaderCallback(void* contents, size_t size, size_t nmemb, void* userp) 
{
    // Parse Content-Length from the headers
    const char* headerPrefix = "Content-Length:";
    size_t headerLength = strlen(headerPrefix);

    // Convert the header data to a string for easier parsing
    std::string header((const char*)contents, size * nmemb);

    // Find the position of the Content-Length header
    size_t pos = header.find(headerPrefix);

    if (pos != std::string::npos) {
        // Extract the value of Content-Length
        long contentLength = std::stol(header.substr(pos + headerLength));

        // Store the content length in userp
        ((HeaderData*)userp)->contentLength = contentLength;
    }

    return size * nmemb;
}

// Get remote file size using Curl APIs
long curlFileSize(const char* url) {
    CURL* curl_handle;
    CURLcode res;
    HeaderData headerData;
    headerData.contentLength = -1L;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    if (curl_handle)
    {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, parseHeaderCallback);
        curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, &headerData); // Pass the address of headerData
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 3L);
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10L);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl_handle);

        if (res == CURLE_OK)
	{
	    printf ("\nFile size: %ld bytes\n",headerData.contentLength);
        }
	else
	{
            printf ("\nError during CURL request: %s\n",curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl_handle);
    }
    else
    {
	printf ("\nError initializing CURL handle\n");
    }

    return headerData.contentLength;
}

// Initialize curl methods and start download
bool curlRead(gpointer Data)
{
  MessageHandlerData *data = (MessageHandlerData *)Data;
  CURL *curl_handle;
  CURLcode res;

  printf ("\nDownloading from %s",m_play_url);

  chunk.memory =  (char *)malloc(1);
  chunk.size = 0;

  filesize = curlFileSize(m_play_url);

  curl_global_init(CURL_GLOBAL_ALL);

  previous_progress_percentage = 0;
  curl_handle = curl_easy_init();

  curl_easy_setopt(curl_handle, CURLOPT_URL, m_play_url);

  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

  curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0);

  if (ReadSize)
      curl_easy_setopt(curl_handle, CURLOPT_MAXFILESIZE, ReadSize * 1024 * 1024);

  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  res = curl_easy_perform(curl_handle);

  if(res != CURLE_OK) 
  {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
    return false;
  }
  else 
  {
    printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
  }

  data->app.length = chunk.size;
  data->app.data = chunk.memory;
  data->app.offset = 0;

  curl_easy_cleanup(curl_handle);
  curl_global_cleanup();
  return true;
}


//Below section is where appsrc feeding is done
static gboolean read_data (MessageHandlerData * data)
{
  GstBuffer *buffer;
  guint len;
  GstFlowReturn ret;

  if (data == NULL || data->app.appsrc == NULL) {
    // Handle the NULL pointer case
    printf("\nReceived NULL pointer");
    return FALSE;
  }
  if ((bufferUnderflowTest) && (total_bytes_fed >= BYTES_THRESHOLD))
  {
     printf("\n reached BYTES_THRESHOLD %lld \n",BYTES_THRESHOLD);
     if (data->terminate)
     {
	 printf ("\nEmitting EOS");
	 g_signal_emit_by_name (data->app.appsrc, "end-of-stream", &ret);
     }
     return FALSE;
  }

  if ((data->app.offset >= data->app.length)) {
    /* we are EOS, send end-of-stream and remove the source */
    g_signal_emit_by_name (data->app.appsrc, "end-of-stream", &ret);
    displayProgressBar("Appsrc Data Push ",(((float)data->app.offset / data->app.length) * 100));
    return FALSE;
  }

  /* read the next chunk */
  buffer = gst_buffer_new ();

  len = CHUNK_SIZE;
  if (data->app.offset + len > data->app.length)
    len = data->app.length - data->app.offset;

  gst_buffer_append_memory (buffer,
      gst_memory_new_wrapped (GST_MEMORY_FLAG_READONLY,
          data->app.data, data->app.length, data->app.offset, len, NULL, NULL));

  float progress_percentage = ((float)data->app.offset / data->app.length) * 100;
  if (abs(progress_percentage - previous_progress_percentage) > 10)
  {
    displayProgressBar("Appsrc Data Push ",progress_percentage);
  }
  g_signal_emit_by_name (data->app.appsrc, "push-buffer", buffer, &ret);
  gst_buffer_unref (buffer);
  if (ret != GST_FLOW_OK) {
    /* some error, stop sending data */
    return FALSE;
  }

  data->app.offset += len;
  total_bytes_fed += len;
  return TRUE;
}

/*************************************************** Appsrc reading end *********************************************/
/********************************************************************************************************************
Purpose:               Setup pipeline with playbin and westerossink
*********************************************************************************************************************/
static void SetupPipeline (MessageHandlerData *data, bool play_after_setup = true)
{
    GstElement *playsink;
    GstStateChangeReturn state_change;
    std::string readFile;

    /*
     * Create the playbin element
     */
    data->playbin = gst_element_factory_make(PLAYBIN_ELEMENT, NULL);
    fail_unless (data->playbin != NULL, "Failed to create 'playbin' element");
    /*
     * Set the url received from argument as the 'uri' for playbin
     */
    assert_failure (data->playbin, m_play_url != NULL, "Playback url should not be NULL");
   
    if (force_appsrc)
	use_appsrc = true;

    // Appsrc implementation skipped for ABR streams
    if (use_appsrc)
    {
        const char *suffix1 = ".m3u8";
        const char *suffix2 = ".mpd";
	if (suffixCompare(m_play_url, suffix1) || suffixCompare(m_play_url, suffix2)) 
	{
            printf("\nAppsrc implementation not supported for ABR streams");
	    printf("\nSwitching to conventional playbin technique\n");
	    use_appsrc = false;
	}
    }
    
    if (use_appsrc)
    {
	const char* filePrefix = "file:";
	size_t maxBytesToRead = ReadSize * 1024 * 1024;
	if (strncmp(m_play_url, filePrefix, strlen(filePrefix)) == 0) 
        {
             const char* file_path = m_play_url + strlen(filePrefix);
             long file_size = GetFileSize(file_path);
             printf ("\nFile Size is %ld bytes",file_size);
             if ((file_size > 104857600) && (!force_appsrc))
             {
                 printf("\nFile size is greater than 100 MB, switching to conventionial approach\n");
                 g_object_set (data->playbin, "uri", m_play_url, NULL);
                 use_appsrc = false;
             }
             else
             {
                 readFromFile(data,file_path,maxBytesToRead);
	         data->app.length = file_size;
             }
        }
        else
        { 
             fail_unless(curlRead(data),"Failed to read from curl API");;
        }
	if (use_appsrc)
	{
            g_object_set (data->playbin, "uri", "appsrc://", NULL);
            g_signal_connect (data->playbin, "source-setup", (GCallback) found_source, data);
            data->app.offset = 0;
            lastDisplayTime = std::chrono::time_point<std::chrono::steady_clock>::min();
	}
    }
    else
    {	    
        g_object_set (data->playbin, "uri", m_play_url, NULL);
    }
    /*
     * Update the current playbin flags to enable Video and Audio Playback
     */
    g_object_get (data->playbin, "flags", &flags, NULL);
    if (only_audio)
    {
        flags = GST_PLAY_FLAG_AUDIO;
#ifdef NATIVE_AUDIO
	flags |= GST_PLAY_FLAG_NATIVE_AUDIO;
#endif
    }
    else if (only_video)
    {
	flags = GST_PLAY_FLAG_VIDEO;
#ifdef NATIVE_VIDEO
        flags |= GST_PLAY_FLAG_NATIVE_VIDEO;
#endif
    }
    else
    {	    
        setflags();
    }
    g_object_set (data->playbin, "flags", flags, NULL);
    if (forward_events)
    {
         /* Forward all events to all sinks */
         playsink = gst_bin_get_by_name(GST_BIN(data->playbin), "playsink");
         g_object_set(playsink, "send-event-mode", 0, NULL);
    }
    /*
     * Set westeros-sink
     */
    data->westerosSink.sink = gst_element_factory_make(WESTEROS_SINK, NULL);
    fail_unless (data->westerosSink.sink != NULL, "Failed to create 'westerossink' element");

    if (forceAudioSink)
    {
	 audiosink = getAudioSink();
    }

    if (!audiosink.empty())
    {
	 printf("\nAudioSink is provided as %s",audiosink.c_str());
         data->audioSink.sink = gst_element_factory_make(audiosink.c_str(), NULL);
	 if (data->audioSink.sink == NULL)
	 {
	     printf("\nUnable to create %s element\nPlaybin will take autoaudiosink\n",audiosink.c_str());
	 }
	 else
	 {
	     g_object_set (data->playbin, "audio-sink", data->audioSink.sink, NULL);
	 }
    }
    /*
     * Link the westeros-sink to playbin
     */
    g_object_set (data->playbin, "video-sink", data->westerosSink.sink, NULL);
    g_object_set (data->playbin, "async-handling", true, NULL);

    g_signal_connect(data->playbin, "element-setup", G_CALLBACK(elementSetupCallback), &data);
    /*
     * Set the first frame received callback
     */
    g_signal_connect(data->westerosSink.sink, "first-video-frame-callback", G_CALLBACK(firstFrameCallback), &firstFrameReceived);
    /*
     * Set the firstFrameReceived variable as false before starting play
     */
    firstFrameReceived= false;

    g_signal_connect(data->westerosSink.sink, "buffer-underflow-callback", G_CALLBACK(bufferUnderflowCallback), &videoUnderflowReceived);

    data->pipelineInitiation = true;
    data->westerosSink.previous_rendered_frames = 0;
    data->audioSink.previous_rendered_frames = 0;

    /*
     * Set playbin to PLAYING
     */
    if (!play_after_setup)
	return;

    GST_FIXME( "Setting to Playing State\n");
    assert_failure (data->playbin, gst_element_set_state (data->playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE);
    GST_FIXME( "Set to Playing State\n");


    if (use_appsrc)
    {
	assert_failure (data->playbin, gst_element_set_state (data->playbin, GST_STATE_PAUSED) !=  GST_STATE_CHANGE_FAILURE);
	printf("\nPushing buffers into appsrc -- Calling read_data\n");
        while(read_data(data));
	assert_failure (data->playbin, gst_element_set_state (data->playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE);
	WaitForOperation;
    }
    else
    {
	do{
            state_change = gst_element_get_state (data->playbin, &(data->cur_state), NULL, 10000000);
        } while (state_change == GST_STATE_CHANGE_ASYNC);
        printf ("\n\n\nPipeline set to : %s  state \n\n\n", gst_element_state_get_name(data->cur_state));
        WaitForOperation;
	assert_failure (data->playbin, data->cur_state == GST_STATE_PLAYING, "Pipeline is not set to playing state");
    }	
    /*
     * Check if the first frame received flag is set
     */
    assert_failure (data->playbin, (only_audio) || (firstFrameReceived == true), "Failed to receive first video frame signal");
    audio_underflow_received =  false;
    videoUnderflowReceived =  false;

    if (checkNewPlay)
        PlaybackValidation(data,5);
    else
	PlaySeconds(data->playbin,5);
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
    GstElement *playbin;
    GstElement *westerosSink;
    GstState cur_state;
    GstStateChangeReturn state_change;
    gint64 currentPosition;

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
 
    assert_failure (playbin,gst_element_set_state (playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE);
    do{
        state_change = gst_element_get_state (playbin, &cur_state, NULL, 10000000);
    } while (state_change == GST_STATE_CHANGE_ASYNC);
    assert_failure (playbin, cur_state == GST_STATE_PLAYING , "Failed to set to playing state");
    printf ("\n********Current state is: %s \n", gst_element_state_get_name(cur_state));

    clock_t start_time = clock();
    const double max_loop_time = 3.0;

    auto start_latency = std::chrono::high_resolution_clock::now();
    while ((clock() - start_time) / CLOCKS_PER_SEC < max_loop_time)
    {
	assert_failure (playbin,gst_element_query_position (playbin, GST_FORMAT_TIME, &currentPosition), "Failed to query the current playback position");
	if ( (int)(currentPosition/GST_SECOND) >= 1)
	{
	     stop_latency = std::chrono::high_resolution_clock::now();
	     //Since playback already happended for 1 second, reduce from stop_latency
	     stop_latency -= std::chrono::seconds(1);
	     break;
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
    GstMessage *message;
    GstBus *bus;
    MessageHandlerData data;

    SetupPipeline(&data);

    /*
     * Wait for 'play_timeout' seconds(received as the input argument) before checking AV status
     */

    if (useProcForFPS)
    {
         char frame_status[BUFFER_SIZE_SHORT] = {'\0'};
         strcat (frame_status, TDK_PATH);
         strcat (frame_status, FRAME_DATA);
         strcat (frame_status, " &");
	 system(frame_status);
    }

    g_object_get (data.westerosSink.sink, "video-height", &height, NULL);
    g_object_get (data.westerosSink.sink, "video-width", &width, NULL);

    printf("\nVideo height = %d\nVideo width = %d", height, width);

    int n_audio;
    g_object_get (data.playbin, "n-audio", &n_audio, NULL);

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
	     assert_failure (data.playbin,to_string(height) == resolution,resolution_value_string);
	     if (checkNewPlay)
		 PlaybackValidation(&data,play_timeout);
	     else
                 PlaySeconds(data.playbin,play_timeout);
	     printf("\nSUCCESS : Resolution is set to %sp successfully\n",resolution.c_str());
	 }
    }
    else
    {
	 if (checkNewPlay)
             PlaybackValidation(&data,play_timeout);
	 else
             PlaySeconds(data.playbin,play_timeout);
    }

    if (ResolutionSwitchTest)
    {
	 assert_failure (data.playbin,ResolutionCount == TOTAL_RESOLUTIONS_COUNT,"\nNot able to play all resolutions\n");
    }
    if (useProcForFPS)
    {
	int rendered_frames;
	rendered_frames = readFramesFromFile();
        printf("\nRendered Frames %d\n",rendered_frames);
    }
    /*
     * Check for AV status if its enabled
     */
    if (true == checkAVStatus)
    {
        is_av_playing = check_for_AV_status();
        assert_failure (data.playbin,is_av_playing == true, "Video is not playing in TV");
    }
    GST_LOG("DETAILS: SUCCESS, Video playing successfully \n");

    if (data.playbin)
    {
       assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_NULL) !=  GST_STATE_CHANGE_FAILURE,"Failed to set to NULL state");
    }
    if (ChannelChangeTest)
    {
       /*
        * Set the url received from argument as the 'channel_url' for playbin
        */
       assert_failure (data.playbin,channel_url != NULL, "Second Channel url should not be NULL");
       g_object_set (data.playbin, "uri", channel_url, NULL);

       /*
        * Set all the required variables before polling for the message
        */
       data.streamStart= FALSE;
       data.terminate = FALSE;

       printf("\nLoading Second Channel\n");

       /*
        * Set playbin to PLAYING
        */
       GST_LOG( "Setting Second Channel to Playing State\n");
       assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE,"Failed to set to playing state");
       GST_LOG( "Second Channel Set to Playing State\n");
       
       bus = gst_element_get_bus (data.playbin);
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
        * Verify that ERROR/EOS messages are not received
        */
       assert_failure (data.playbin,FALSE == data.terminate, "Unexpected error or End of Stream received\n");
       /*
        * Verify that STREAM_START message is received
        */
       assert_failure (data.playbin,TRUE == data.streamStart, "Unable to obtain message indicating start of a new stream\n");
       /*
        * Check for AV status if its enabled
        */
       if (true == checkAVStatus)
       {
          is_av_playing = check_for_AV_status();
          assert_failure (data.playbin,is_av_playing == true, "Video is not playing in TV");
       }
       GST_LOG("DETAILS: SUCCESS, Video playing successfully \n");
       /*
        * Wait for 'SecondChannelTimeout' seconds(received as the input argument) for video to play
        */

       data.pipelineInitiation = true;
       if (checkNewPlay)
	    PlaybackValidation(&data,SecondChannelTimeout);
       else
            PlaySeconds(data.playbin,SecondChannelTimeout);

       if (data.playbin)
       {
          gst_element_set_state (data.playbin, GST_STATE_NULL);
       }
       gst_object_unref(bus);
    }
    /*
     * Cleanup after use
     */
    gst_object_unref (data.playbin);
}
GST_END_TEST;

/********************************************************************************************************************
 * Purpose      : Test to change the state of pipeline from play to pause
 * Parameters   : Playback url
 ********************************************************************************************************************/
GST_START_TEST (test_play_pause_pipeline)
{
    bool is_av_playing = false;
    GstStateChangeReturn state_change;
    MessageHandlerData data;

    SetupPipeline(&data);
    
    if (!only_audio)
    {
         g_object_get (data.westerosSink.sink, "video-height", &height, NULL);
         g_object_get (data.westerosSink.sink, "video-width", &width, NULL);
    
         printf("\nVideo height = %d\nVideo width = %d", height, width);
    }

    if (checkNewPlay)
        PlaybackValidation(&data,play_timeout);
    else
        PlaySeconds(data.playbin,play_timeout);
    /*
     * Check for AV status if its enabled
     */
    if (true == checkAVStatus)
    {
        is_av_playing = check_for_AV_status();
        assert_failure (data.playbin,is_av_playing == true, "Video is not playing in TV");
    }
    printf ("\nDETAILS: SUCCESS, Playback is successfull\n");

    auto latency_start = std::chrono::high_resolution_clock::now();
    auto latency_stop =  std::chrono::high_resolution_clock::now();
    auto latency_chrono = std::chrono::duration_cast<std::chrono::milliseconds>(latency_stop - latency_start);

    /*
     * Set pipeline to PAUSED
     */
    assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &data.previousposition), "Failed to query the current playback position");
    gst_element_set_state (data.playbin, GST_STATE_PAUSED);
    do{
        state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, 10000000);
    } while (state_change == GST_STATE_CHANGE_ASYNC);
    printf ("\n********Current state is: %s \n", gst_element_state_get_name(data.cur_state));
    assert_failure (data.playbin, data.cur_state == GST_STATE_PAUSED);
    printf ("\nDETAILS: SUCCESS, Current state is: %s \n", gst_element_state_get_name(data.cur_state));

    if (checkNewPlay)
        PlaybackValidation(&data,play_timeout);
    else
        PlaySeconds(data.playbin,play_timeout);

    assert_failure (data.playbin, gst_element_get_state (data.playbin, &data.cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS);
    if ((data.cur_state != GST_STATE_PLAYING))
    {
	assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &data.previousposition), "Failed to query the current playback position");    
        assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "unable to pause");
        do{
             state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, 10000000);
        } while (state_change == GST_STATE_CHANGE_ASYNC);
	printf ("\n********Current state is: %s \n", gst_element_state_get_name(data.cur_state));
    }

    assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &data.previousposition), "Failed to query the current playback position");
    if (checkNewPlay)
        PlaybackValidation(&data,play_timeout);
    else
        PlaySeconds(data.playbin,play_timeout);

    if (data.playbin)
    {
       gst_element_set_state(data.playbin, GST_STATE_NULL);
    }

    /*
     * Cleanup after use
     */
    gst_object_unref (data.playbin);
}
GST_END_TEST;

/********************************************************************************************************************
 * Purpose      : Test to query total duration of video/audio stream via playbin
 * Parameters   : Playback url
 ********************************************************************************************************************/
GST_START_TEST (test_playback_duration)
{
    MessageHandlerData data;

    if (only_audio)
    {
	checkPTS = false;
	use_westerossink_fps = false;
    }

    SetupPipeline(&data);

    gint64 duration = -1;
    gst_element_query_duration(data.playbin, GST_FORMAT_TIME, &duration);

    assert_failure(data.playbin, (duration != -1), "Failed to query pipeline duration");
   
    if (data.playbin)
    {
       gst_element_set_state(data.playbin, GST_STATE_NULL);
    }
    gst_object_unref (data.playbin);

    g_print("Duration: %" G_GINT64_FORMAT ":%02" G_GINT64_FORMAT ".%03" G_GINT64_FORMAT "\n",
        GST_TIME_AS_SECONDS(duration) / 60,  // Minutes
        GST_TIME_AS_SECONDS(duration) % 60,  // Seconds
        GST_TIME_AS_MSECONDS(duration) % 1000);  // Milliseconds
    

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
    gint64 seekPosition;
    bool seeked = false;
    int Seek_time_threshold = 5;
    GstStateChangeReturn state_change;
    MessageHandlerData data;

    if (use_appsrc)
    {
        printf("\nUnderflow stream test - switching to conventionial approach\n");
        use_appsrc = false;
    }
    SetupPipeline(&data);

    bufferUnderflowTest = true;
    assert_failure (data.playbin,videoEnd != 0,"videoEnd point is not given");
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
	 if (checkNewPlay)
             PlaybackValidation(&data,play_timeout + videoEnd);
	 else
             PlaySeconds(data.playbin,play_timeout + videoEnd);
    }
    else
    {
	 /*
	  * Wait for 'videoEnd' seconds(received as the input argument) to receive underflow signal from westerossink
	  */
	 if (checkNewPlay)
	     PlaybackValidation(&data,videoEnd);
	 else
             PlaySeconds(data.playbin,videoEnd);
    }

    assert_failure (data.playbin,videoUnderflowReceived == true, "Failed to receive buffer underflow signal");

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

    assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_PAUSED) !=  GST_STATE_CHANGE_FAILURE, "unable to pause");
    do{
       //Waiting for state change
       state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, 10000000);
    } while (state_change == GST_STATE_CHANGE_ASYNC);

    assert_failure (data.playbin, gst_element_get_state (data.playbin, &data.cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS);
    assert_failure (data.playbin, data.cur_state == GST_STATE_PAUSED);
    printf("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));
    GST_LOG("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));
    assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &(data.previousposition)), "Failed to query the current playback position");
    if (checkNewPlay)
        PlaybackValidation(&data,5);
    else
        PlaySeconds(data.playbin,5);

    /*
     * Seek to the 'videoStart' point (received as the input argument) to fill the video buffer
     * And start playback
     */
    assert_failure (data.playbin,videoStart != 0,"videoStart point is not given");
    assert_failure (data.playbin,videoStart > videoEnd,"videoEnd and videoStart params are not correct");
    seekPosition = GST_SECOND * (videoStart + 2);
    seekOperation = true;
    seekSeconds = seekPosition/GST_SECOND;
    assert_failure (data.playbin,gst_element_seek (data.playbin, NORMAL_PLAYBACK_RATE, GST_FORMAT_TIME,
                                   GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, seekPosition,
                                   GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE), "Failed to seek");

    start = std::chrono::steady_clock::now();
    while(!seeked)
    {
       //Check if seek happened
       assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &(data.currentPosition)),
                                         "Failed to query the current playback position");
       //Added GST_SECOND buffer time between currentPosition and seekPosition
       if (abs( data.currentPosition - seekPosition) <= (GST_SECOND))
       {
           seeked = TRUE;
       }
       if (std::chrono::steady_clock::now() - start > std::chrono::seconds(Seek_time_threshold))
           break;
    }

    assert_failure (data.playbin,TRUE == seeked, "Seek Unsuccessfull\n");
    //Convert time to seconds
    data.currentPosition /= GST_SECOND;
    seekPosition /= GST_SECOND;
    printf("\nSEEK SUCCESSFULL :  CurrentPosition %lld seconds, SeekPosition %lld seconds\n", data.currentPosition, seekPosition);

    state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, GST_SECOND);
    assert_failure (data.playbin,state_change != GST_STATE_CHANGE_FAILURE, "Failed to get current playbin state");

    if ((data.cur_state == GST_STATE_PAUSED)  && (state_change != GST_STATE_CHANGE_ASYNC))
    {

        /*
         * Set playbin to PLAYING
         */
         GST_FIXME( "Setting to Playing State\n");
         assert_failure (data.playbin,gst_element_set_state(data.playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "unable to set to play");
         GST_FIXME( "Set to Playing State\n");
         do{
              //Waiting for state change
              state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, 10000000);
         } while (state_change == GST_STATE_CHANGE_ASYNC);

         printf("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));
	 assert_failure (data.playbin, gst_element_get_state (data.playbin, &data.cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS);
         assert_failure (data.playbin, data.cur_state == GST_STATE_PLAYING);
         GST_LOG("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));
    }

    data.previousposition = seekPosition*GST_SECOND;
    if (checkNewPlay)
        PlaybackValidation(&data,play_timeout);
    else
        PlaySeconds(data.playbin,play_timeout);

    printf ("\nDETAILS: SUCCESS Playback after seeking to Video Point after buffer underflow is successfull\n");
EXIT :
    /* Check for AV status if its enabled
     */
    if (true == checkAVStatus)
    {
        is_av_playing = check_for_AV_status();
        assert_failure (data.playbin,is_av_playing == true, "Video is not playing in TV");
    }
    if (data.playbin)
    {
       gst_element_set_state(data.playbin, GST_STATE_NULL);
    }
    /*
     * Cleanup after use
     */
    gst_object_unref (data.playbin);
}
GST_END_TEST;


/********************************************************************************************************************
 * Purpose      : Test to check that EOS message is received
 * Parameters   : Playback url
 ********************************************************************************************************************/
GST_START_TEST (test_EOS)
{
    bool is_av_playing = false;
    GstMessage *message;
    use_westerossink_fps = false;    
    GstBus *bus;
    bool received_EOS = false;
    MessageHandlerData data;

    SetupPipeline(&data);

    /*
     * Check for AV status if its enabled
     */
    if (true == checkAVStatus)
    {
        is_av_playing = check_for_AV_status();
        assert_failure (data.playbin,is_av_playing == true, "Video is not playing in TV");
    }
    printf ("DETAILS: SUCCESS, Video playing successfully \n");

    /*
     * Polling the bus for messages
     */
    bus = gst_element_get_bus (data.playbin);
    assert_failure (data.playbin,bus != NULL,"Failed to get bus");
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
    assert_failure (data.playbin,received_EOS == true, "Failed to recieve EOS message");
    printf ("EOS Received\n");
    gst_message_unref (message);


    if (data.playbin)
    {
       gst_element_set_state(data.playbin, GST_STATE_NULL);
    }
    /*
     * Cleanup after use
     */
    gst_object_unref (data.playbin);
}
GST_END_TEST;


GST_START_TEST (test_frameDrop)
{
    GstElement *pipeline, *westerosSink;
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

    g_object_get (pipeline, "flags", &flags, NULL);
    setflags();
    g_object_set (pipeline, "flags", flags, NULL); 

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
    startPosition = currentPosition;
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
    g_object_get (westerosSink,"stats",&structure,NULL);
    gst_structure_get_uint64(structure, "rendered", &rendered_frames);
    previous_rendered_frames = rendered_frames;
    if (useProcForFPS)
    {
        previous_rendered_frames_proc = readFramesFromFile();
    }


    do
    {
        Sleep(1);
        runLoop = false;
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
	    {
		rendered_frames = readFramesFromFile();
                if(rendered_frames < previous_rendered_frames_proc)
                   jump_buffer -=1;
	    }
	    else
            {
		if(rendered_frames < previous_rendered_frames)
		   jump_buffer -=1;
	    }
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



    if ((int)rendered_frames < expected_rendered_frames)
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
    use_westerossink_fps = false;
    MessageHandlerData data;
    char audio_switch_error_string[100];
    GstTagList *tags;
    strcat (audio_change_test, TDK_PATH);
    strcat (audio_change_test, "/audio_change_log");
    filePointer = fopen(audio_change_test, "w");
    assert_failure (data.playbin,filePointer != NULL,"Unable to open filePointer for writing");

    if (use_appsrc)
    {
        printf("\nMultiple Codecs available in stream, so skipping appsrc approach and going with conventional approach\n");
        use_appsrc = false;
    }

    SetupPipeline(&data);

    if (AudioPTSCheckAvailable)
    {
         char audio_status[BUFFER_SIZE_SHORT] = {'\0'};
         strcat (audio_status, TDK_PATH);
         strcat (audio_status, AUDIO_DATA);
         strcat (audio_status, " &");
	     system(audio_status);
    }

    getStreamProperties(&data);
    assert_failure (data.playbin,1 < data.n_audio,"Stream has only 1 audio stream. Audio Change requires minimum of two audio streams");
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
		 if (checkNewPlay)
                     PlaybackValidation(&data,2);
                 else
                     PlaySeconds(data.playbin,2);
	     }
             printf("\nSwitching to audio stream %d\n", index);
             printf("\nSetting current-audio to %d\n",index);
             g_object_set (data.playbin, "current-audio", index, NULL);
             // Waiting for audio switch
             g_object_get (data.playbin, "current-audio", &data.current_audio, NULL);
	     sprintf(audio_switch_error_string,"FAILED : Unable to switch to %d audio stream",index);
             assert_failure (data.playbin,data.current_audio == index, audio_switch_error_string);
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
	     assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &(data.currentPosition)), "Failed to query the current playback position");
             if (Flush_Pipeline)
             {
                 flushPipeline(&data);
             }
             printf("\nSUCCESS : Switched to audio stream %d, Playing for %d seconds\n",index,play_timeout);
         }
	 if (checkNewPlay)
             PlaybackValidation(&data,play_timeout);
         else
             PlaySeconds(data.playbin,play_timeout);

         assert_failure (data.playbin,audio_pts_error == false, "\nERROR : AUDIO PTS ERROR OBSERVED\n");
         assert_failure (data.playbin,audio_underflow_received == false, "\nERROR : AUDIO UNDERFLOW OBSERVED\n");
         if (AudioPTSCheckAvailable)
         {
             char audio_pts_status[BUFFER_SIZE_SHORT] = {'\0'};
             strcat (audio_pts_status, TDK_PATH);
             strcat (audio_pts_status, AUDIO_PTS_ERROR_FILE);
             strcat (audio_pts_status, " &");
             assert_failure (data.playbin,access(audio_pts_status, 0) == 0,"ERROR : AUDIO PTS ERROR OBSERVED\n");
         }
    }
    /*
     * Check for AV status if its enabled
     */
    if (true == checkAVStatus)
    {
        is_av_playing = check_for_AV_status();
        assert_failure (data.playbin,is_av_playing == true, "Video is not playing in TV");
    }
    GST_LOG("DETAILS: SUCCESS, Video playing successfully for all audio streams\n");


    if (data.playbin)
    {
       assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_NULL) !=  GST_STATE_CHANGE_FAILURE);
    }
    /*
     * Cleanup after use
     */
    fclose(filePointer);
    gst_object_unref (data.playbin);
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
    gint64 seekPosition;
    bool seeked = false;
    int Seek_time_threshold = 5;
    GstStateChangeReturn state_change;
    int runSeconds;
    MessageHandlerData data;

    if (use_appsrc)
    {
        printf("\nUnderflow stream test - switching to conventionial approach\n");
        use_appsrc = false;
    }

    SetupPipeline(&data);
    
    bufferUnderflowTest = true;
    audio_underflow_received = false;


    assert_failure (data.playbin,audioEnd != 0,"audioEnd point is not given");
    if (play_without_audio)
    {
         if (avsync_enabled)
         {
	      if (checkNewPlay)
	           PlaybackValidation(&data,play_timeout + audioEnd);
    	      else
                   PlaySeconds(data.playbin,play_timeout + audioEnd);
              runSeconds = 0;
              g_object_get (data.westerosSink.sink,"stats",&data.westerosSink.structure,NULL);
              gst_structure_get_uint64(data.westerosSink.structure, "rendered", &data.westerosSink.rendered_frames);
              data.westerosSink.previous_rendered_frames = data.westerosSink.rendered_frames;
              while ( runSeconds < play_timeout )
              {
                  Sleep(1);
                  runSeconds++ ;
                  g_object_get (data.westerosSink.sink,"stats",&data.westerosSink.structure,NULL);
                  gst_structure_get_uint64(data.westerosSink.structure, "rendered", &data.westerosSink.rendered_frames);
                  printf ("\nvideo frames");
                  printf(" Rendered: %" G_GUINT64_FORMAT, data.westerosSink.rendered_frames);
	          printf("\n");
                  assert_failure(data.playbin, data.westerosSink.previous_rendered_frames == data.westerosSink.rendered_frames,"Video is not stopped as expected");
                  data.westerosSink.previous_rendered_frames  = data.westerosSink.rendered_frames;
              };
         }
         else
         {
              bufferUnderflowTest = false;
              ignorePlayJump = true;
              use_westerossink_fps = true;
              if (checkNewPlay)
                  PlaybackValidation(&data,play_timeout + audioEnd);
              else
                  PlaySeconds(data.playbin,play_timeout + audioEnd);
         }
    }
    else
    {
	if (checkNewPlay)
             PlaybackValidation(&data,audioEnd);
        else
             PlaySeconds(data.playbin,audioEnd);
    }

    assert_failure (data.playbin,audio_underflow_received_global == true, "Failed to receive buffer underflow signal");
    	
    printf ("\nDETAILS: SUCCESS, Received buffer underflow signal as expected\n");

    
    if (checkSignalTest || play_without_audio)
         goto EXIT;
	
    bufferUnderflowTest = false;

    assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_PAUSED) !=  GST_STATE_CHANGE_FAILURE, "unable to pause");
    do{
       //Waiting for state change
       state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, 10000000);
    } while (state_change == GST_STATE_CHANGE_ASYNC);

    assert_failure (data.playbin, gst_element_get_state (data.playbin, &data.cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS);
    assert_failure (data.playbin, data.cur_state == GST_STATE_PAUSED);
    printf("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));
    GST_LOG("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));
    assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &(data.previousposition)), "Failed to query the current playback position");
    if (checkNewPlay)
        PlaybackValidation(&data,5);
    else
        PlaySeconds(data.playbin,5);
    
    assert_failure (data.playbin, gst_element_get_state (data.playbin, &data.cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS);
    assert_failure (data.playbin, data.cur_state == GST_STATE_PAUSED);
    printf("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));
    GST_LOG("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));

    /*
     * Seek to the 'audioStart' point (received as the input argument) to fill the audio buffer
     * And start playback
     */
    assert_failure (data.playbin,audioStart != 0,"audioStart point is not given");
    assert_failure (data.playbin,audioStart > audioEnd,"audioEnd and audioStart params are not correct");
    seekPosition = GST_SECOND * (audioStart + 2);
    seekOperation = true;
    seekSeconds = seekPosition/GST_SECOND;
    assert_failure (data.playbin,gst_element_seek (data.playbin, NORMAL_PLAYBACK_RATE, GST_FORMAT_TIME,
                                   GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, seekPosition,
                                   GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE), "Failed to seek");

    start = std::chrono::steady_clock::now();
    while(!seeked)
    {
       //Check if seek happened
       assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &(data.currentPosition)),
                                         "Failed to query the current playback position");
       //Added GST_SECOND buffer time between currentPosition and seekPosition
       if (abs( data.currentPosition - seekPosition) <= (GST_SECOND))
       {
           seeked = TRUE;
       }
       if (std::chrono::steady_clock::now() - start > std::chrono::seconds(Seek_time_threshold))
           break;
    }

    assert_failure (data.playbin,TRUE == seeked, "Seek Unsuccessfull\n");
    //Convert time to seconds
    data.currentPosition /= GST_SECOND;
    seekPosition /= GST_SECOND;
	printf("\nSEEK SUCCESSFULL :  CurrentPosition %lld seconds, SeekPosition %lld seconds\n", data.currentPosition, seekPosition);

    state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, GST_SECOND);
    assert_failure (data.playbin,state_change != GST_STATE_CHANGE_FAILURE, "Failed to get current playbin state");

    if ((data.cur_state == GST_STATE_PAUSED)  && (state_change != GST_STATE_CHANGE_ASYNC))
    {

        /*
         * Set playbin to PLAYING
         */
         GST_FIXME( "Setting to Playing State\n");
         assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "unable to pause");
         GST_FIXME( "Set to Playing State\n");
         do{
              //Waiting for state change
              state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, 10000000);
         } while (state_change == GST_STATE_CHANGE_ASYNC);

         printf("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));
	 assert_failure (data.playbin, gst_element_get_state (data.playbin, &data.cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS);
         assert_failure (data.playbin, data.cur_state == GST_STATE_PLAYING);
         GST_LOG("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));
    }

    data.previousposition = seekPosition*GST_SECOND;
    if (checkNewPlay)
        PlaybackValidation(&data,play_timeout);
    else
        PlaySeconds(data.playbin,play_timeout);

    printf ("\nDETAILS: SUCCESS Playback after seeking to Audio Point after buffer underflow is successfull\n");

EXIT :
    /* Check for AV status if its enabled
     */
    if (true == checkAVStatus)
    {
        is_av_playing = check_for_AV_status();
        assert_failure (data.playbin,is_av_playing == true, "Video is not playing in TV");
    }
    if (data.playbin)
    {
       gst_element_set_state(data.playbin, GST_STATE_NULL);
    }

    gst_object_unref (data.playbin);

}
GST_END_TEST;

GST_START_TEST(test_only_audio)
{
	MessageHandlerData data;
	checkPTS = false;
	use_westerossink_fps = false;
	only_audio = true;

   	SetupPipeline(&data);

	if (checkNewPlay)
            PlaybackValidation(&data,play_timeout);
        else
            PlaySeconds(data.playbin,play_timeout);

	if (data.playbin)
    	{
       		assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_NULL) !=  GST_STATE_CHANGE_FAILURE);
    	}
	
	gst_object_unref (data.playbin);
}
GST_END_TEST;

GST_START_TEST (test_video_bitrate)
{
    MessageHandlerData data;

    SetupPipeline(&data);

    assert_failure (data.playbin,connection_speed != 0,"connection speed is not given");
    g_object_set (data.playbin, "connection_speed", connection_speed, NULL);
    printf("\nConnection Speed: %" G_GUINT64_FORMAT, connection_speed);


    g_object_get (data.westerosSink.sink, "video-height", &height, NULL);
    g_object_get (data.westerosSink.sink, "video-width", &width, NULL);

    printf("\nVideo height = %d\nVideo width = %d", height, width);

    if (checkNewPlay)
        PlaybackValidation(&data,play_timeout);
    else
        PlaySeconds(data.playbin,play_timeout);

    if (data.playbin)
    {
        assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_NULL) !=  GST_STATE_CHANGE_FAILURE);
    }

    gst_object_unref (data.playbin);
}
GST_END_TEST;

/********************************************************************************************************************
 * Purpose      : Test to do check the underflow handling capability via appsrc
 * Parameters   : Playback URL
 ********************************************************************************************************************/
GST_START_TEST (test_appsrc_underflow)
{

    MessageHandlerData data;
    bufferUnderflowTest = true;
    play_timeout = 35;
    force_appsrc = true;
    SetupPipeline(&data,false);

    assert_failure (data.playbin, gst_element_set_state (data.playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE);

    data.app.offset = 0;
    while(read_data(&data));
    WaitForOperation;

    PlaybackValidation(&data,play_timeout);

    if (checkSignalTest)
	 goto EXIT;
    if (video_underflow_test)
	 assert_failure (data.playbin,videoUnderflowReceived == true, "Failed to receive buffer underflow signal");
    if (audio_underflow_test)
	 assert_failure (data.playbin,audio_underflow_received_global == true, "Failed to receive buffer underflow signal");

    BYTES_THRESHOLD *= 2;
    printf ("\nUpdating bytestThreshold to %lld",BYTES_THRESHOLD);
    printf ("\nPushing buffers with updated bytesThreshold\n");
    videoUnderflowReceived = false;
    audio_underflow_received_global = false;
    data.terminate = true;
    while(read_data(&data));

    PlaybackValidation(&data,play_timeout);

EXIT:
    if (data.playbin)
    {
        assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_NULL) !=  GST_STATE_CHANGE_FAILURE);
    }

    gst_object_unref (data.playbin);

}
GST_END_TEST;

/********************************************************************************************************************
 * Purpose      : Test to do check the Color depth of video using playbin element and Caps
 * Parameters   : Playback URL
 ********************************************************************************************************************/
GST_START_TEST (test_color_depth)
{

    MessageHandlerData data;

    SetupPipeline(&data);

    if (checkNewPlay)
        PlaybackValidation(&data,play_timeout);
    else
        PlaySeconds(data.playbin,play_timeout);

    string Bit_depth_1;

    if (brcm)
    {	    
     	Bit_depth_1 = executecommand (data.playbin,"cat /proc/brcm/video_decoder | grep Source | cut -d ' ' -f5 | tr -d ' '");
        Bit_depth_got = atoi(Bit_depth_1.c_str() );
    }
    else if (rtk)
    {
        Bit_depth_1 = executecommand (data.playbin,"sh /lib/rdk/get_avstatus.sh | grep LumaBitDepth | cut -d ' ' -f3 | tr -d ' '");
        Bit_depth_got = atoi(Bit_depth_1.c_str() );
    }	    
    else
    	print_pad_capabilities (data.westerosSink.sink, "sink");


    assert_failure (data.playbin,bit_depth != 0,"bit depth is not given");

    printf("\nBit_Depth_Got = %d\n", Bit_depth_got);
    
    assert_failure (data.playbin,(bit_depth == Bit_depth_got), "Given bit_depth was not matched");

    if (data.playbin)
    {
	    assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_NULL) !=  GST_STATE_CHANGE_FAILURE);
    }

    /*
     * Cleanup after use
     */
    gst_object_unref (data.playbin);

}
GST_END_TEST;

/********************************************************************************************************************
 * Purpose      : Test to do check the volume of the audio playback using playbin element and westeros-sink
 * Parameters   : Playback URL
 ********************************************************************************************************************/

GST_START_TEST (test_audio_volume)
{

    gdouble volume_set_array[5] = {1,0.75,0.5,0.25,0};
    MessageHandlerData data;

    SetupPipeline(&data);

    if (!volume_stress)
	 assert_failure (data.playbin,volume_set != 1000,"volume set is not given");

    if (!volume_stress)
    {
	if (checkNewPlay)
            PlaybackValidation(&data,5);
        else
            PlaySeconds(data.playbin,5);

    	play_set_relative_volume (data.playbin);

	if (checkNewPlay)
            PlaybackValidation(&data,10);
        else
            PlaySeconds(data.playbin,10);
    }
    else
    {
	    for (int i=0 ; i < static_cast<int>(sizeof(volume_set_array)/sizeof(*volume_set_array)); i++)
	    {
		    volume_set = volume_set_array[i];
		    if (checkNewPlay)
        		PlaybackValidation(&data,5);
    		    else
        		PlaySeconds(data.playbin,5);

		    play_set_relative_volume (data.playbin);

		    if (checkNewPlay)
        		PlaybackValidation(&data,10);
    		   else
		        PlaySeconds(data.playbin,10);
	    }
    }

    if (data.playbin)
    {
        assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_NULL) !=  GST_STATE_CHANGE_FAILURE);
    }

    /*
     * Cleanup after use
     */
    gst_object_unref (data.playbin);

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
    else if (strcmp ("test_buffer_underflow_signal", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_buffer_underflow);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_buffer_underflow_playback", tcname) == 0)
    {
       checkSignalTest = false;
       tcase_add_test (tc_chain, test_buffer_underflow);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_audio_underflow_signal", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_audio_underflow);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_audio_underflow_playback", tcname) == 0)
    {
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
    else if (strcmp ("test_resolution_up", tcname) == 0)
    {
       ResolutionSwitchTest = true;
       if (use_appsrc)
       {
           printf ("\nMultiple resolutions present in video, caps can't be set to appsrc\nSwitching to conventionial approach\n");
           use_appsrc =false;
       }
       tcase_add_test (tc_chain, test_generic_playback);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_resolution_down", tcname) == 0)
    {
       ResolutionSwitchTest = true;
       if (use_appsrc)
       {
           printf ("\nMultiple resolutions present in video, caps can't be set to appsrc\nSwitching to conventionial approach\n");
           use_appsrc =false;
       }
       resolution_test_up = false;
       tcase_add_test (tc_chain, test_generic_playback);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_audio_sampling_rate", tcname) == 0)
    {
       checkAudioSamplingrate = true;
       use_audioSink = true;
       tcase_add_test (tc_chain, test_play_pause_pipeline);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_only_audio", tcname) == 0)
    {
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
    else if (strcmp ("test_audio_volume", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_audio_volume);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_audio_volume_stress", tcname) == 0)
    {
       volume_stress = true;
       tcase_add_test (tc_chain, test_audio_volume);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_audio_duration", tcname) == 0)
    {
       only_audio = true;
       tcase_add_test (tc_chain, test_playback_duration);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_playback_duration", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_playback_duration);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_appsrc_video_underflow_signal", tcname) == 0)
    {
       video_underflow_test = true;
       only_video = true;
       use_audioSink = false;
       checkSignalTest = true;
       tcase_add_test (tc_chain, test_appsrc_underflow);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_appsrc_audio_underflow_signal", tcname) == 0)
    {
       audio_underflow_test = true;
       only_audio = true;
       use_westerossink_fps = false;
       checkPTS = false;
       checkSignalTest = true;
       tcase_add_test (tc_chain, test_appsrc_underflow);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_appsrc_video_underflow", tcname) == 0)
    {
       video_underflow_test = true;
       only_video = true;
       use_audioSink = false;
       checkSignalTest = false;
       tcase_add_test (tc_chain, test_appsrc_underflow);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_appsrc_audio_underflow", tcname) == 0)
    {
       audio_underflow_test = true;
       only_audio = true;
       use_westerossink_fps = false;
       checkPTS = false;
       checkNewPlay = true;
       checkSignalTest = false;
       tcase_add_test (tc_chain, test_appsrc_underflow);
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

/********************************************************************************************************************
 * Purpose      : To check if environmental variable is already set
 * Parameters   : Environmental Variable
 ********************************************************************************************************************/
bool isEnvVarSet(const char* varName) {
    return getenv(varName) != nullptr;
}

/********************************************************************************************************************
 * Purpose      : To set environmental variables
 * Parameters   : Environmental Variable
 *                Value for environmental variable
 ********************************************************************************************************************/
void setEnvironmentVariable(const char* varName, const char* varValue)
{
    if (isEnvVarSet(varName))
    {
        printf("Environment variable already set: %s=%s\n", varName, getenv(varName));
    }
    else
    {
        if (setenv(varName, varValue, 1) != 0)
	{
            printf("Error setting environment variable: %s\n", varName);
        }
	else
	{
            printf("Set environment variable: %s=%s\n", varName, varValue);
        }
    }
}

/********************************************************************************************************************
 * Purpose      : To read from ENV_FILE and set the corresponding environmental variables
 ********************************************************************************************************************/
int setVariables()
{
    FILE* inputFile = fopen(ENV_FILE, "r");
    if (inputFile)
    {
        char line[256];
        while (fgets(line, sizeof(line), inputFile))
        {
             line[strcspn(line, "\n")] = '\0';
             if (strncmp(line, "export ", 7) == 0)
             {
		 char* varName = line + 7;
		 char* equalsPos = strchr(varName, '=');
		 if (equalsPos != nullptr)
		 {
		     *equalsPos = '\0';
		     setEnvironmentVariable(varName, equalsPos + 1);
		 }
             }
             else
             {
                 char* equalsPos = strchr(line, '=');
                 if (equalsPos != nullptr)
                 {
                     *equalsPos = '\0';
                     setEnvironmentVariable(line, equalsPos + 1);
                 }
             }
         };
         fclose(inputFile);
	 return 1;
     }
     else
     {
	 printf ("\nUnable to open %s file for reading\n", ENV_FILE);
	 return 0;
     }
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
    setVariables();
    if (getenv ("TDK_PATH") != NULL)
    {
    	strcpy (TDK_PATH, getenv ("TDK_PATH"));
    }
    else
    {
	if (access(ENV_FILE, F_OK) == 0)
	{
	    if (!(setVariables()))
		goto exit;
	}
	else
	{
	    GST_ERROR ("Environment variable TDK_PATH should be set!!!!");
	    printf ("Environment variable TDK_PATH is not set!!!!\n");
	    printf ("Environment variables can be set in /opt/TDK.env\n");
	    goto exit;
	}
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
	if (strcmp ("test_channel_change_playback", tcname) == 0)
	{
      	    strcpy(channel_url,argv[3]);
            arg = 4;
	}
	else
	{
	    arg = 3;
	}
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
	    (strcmp ("test_appsrc_video_underflow", tcname) == 0) ||
	    (strcmp ("test_appsrc_audio_underflow", tcname) == 0) ||
	    (strcmp ("test_appsrc_video_underflow_signal", tcname) == 0) ||
	    (strcmp ("test_appsrc_audio_underflow_signal", tcname) == 0) ||
            (strcmp ("test_EOS", tcname) == 0) ||
	    (strcmp ("test_audio_sampling_rate", tcname) == 0) ||
	    (strcmp ("test_only_audio", tcname) == 0) ||
	    (strcmp ("test_video_bitrate", tcname) == 0) ||
	    (strcmp ("test_color_depth", tcname) == 0) ||
	    (strcmp ("test_audio_volume", tcname) == 0) ||
	    (strcmp ("test_audio_volume_stress", tcname) == 0) ||
	    (strcmp ("test_playback_duration", tcname) == 0) ||
	    (strcmp ("test_channel_change_playback", tcname) == 0) ||
	    (strcmp ("test_audio_duration", tcname) == 0))
	{
	    strcpy(m_play_url,argv[2]);

            for (; arg < argc; arg++)
            {
                if (strcmp ("checkavstatus=yes", argv[arg]) == 0)
                {
                    checkAVStatus = true;
                }
		// For Channel Change test getting channel timeouts has been handled separately
                if ((strstr (argv[arg], "timeout=") != NULL) && (strcmp ("test_channel_change_playback", tcname) != 0))
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
		if (strstr (argv[arg], "volume_set=") != NULL)
                {
                    strtok (argv[arg], "=");
                    volume_set = stod (strtok (NULL, "="));
                }
		if (strcmp ("checkAudioFPS=no", argv[arg]) == 0)
                {
                    use_audioSink = false;
                }
		if (strcmp ("checkASR=yes", argv[arg]) == 0)
                {
                    checkAudioSamplingrate = true;
                }
		if (strcmp ("avsync_enabled", argv[arg]) == 0)
        	{
           	    avsync_enabled = true;
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
		if (strstr (argv[arg], "ignoreError") != NULL)
                {
                    ignoreError = true;
		}
		if (strstr (argv[arg], "audioSink=") != NULL)
                {
                    strtok (argv[arg], "=");
                    audiosink = (strtok (NULL, "="));
                }
                if (strcmp ("forwardEvents=no", argv[arg]) == 0)
                {
                    forward_events = false;
                }
		if (strcmp ("validateFullPlayback", argv[arg]) == 0)
                {
                    checkNewPlay = true;
                }
		if (strcmp ("checkEachSecondPlayback", argv[arg]) == 0)
		{
		    checkEachSecondPlayback = true;
		}
		if (strcmp ("forceAudioSink", argv[arg]) == 0)
                {
                    forceAudioSink = true;
                }
		if (strcmp ("test_channel_change_playback", tcname) == 0)
		{
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
		if (strstr (argv[arg], "readSize=") != NULL)
                {
                    strtok (argv[arg], "=");
                    ReadSize = atoi (strtok (NULL, "="));
                }
		if (strstr (argv[arg], "underflow_threshold=") != NULL)
                {
                    strtok (argv[arg], "=");
                    BYTES_THRESHOLD = atoi (strtok (NULL, "="));
                }
		if (strcmp ("use_appsrc", argv[arg]) == 0)
                {
                    use_appsrc = true;
                }
		if (strcmp ("only_audio", argv[arg]) == 0)
                {
                    only_audio = true;
                }
            }

            printf ("\nArg : TestCase Name: %s \n", tcname);
            printf ("\nArg : Playback url: %s \n", m_play_url);

	    if (strcmp ("test_channel_change_playback", tcname) == 0)
	    {
		printf ("\nArg : TestCase Name: %s \n", tcname);
                printf ("\nArg : First Channel: %s \n", m_play_url);
                printf ("\nArg : FirstChannel timeout: %d \n", play_timeout);
                printf ("\nArg : Second Channel : %s \n",channel_url);
                printf ("\nArg : SecondChannel timeout: %d\n", SecondChannelTimeout);
            }

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


