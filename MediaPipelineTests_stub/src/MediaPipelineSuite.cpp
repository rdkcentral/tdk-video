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
#include "tinyxml2.h"
#include <dlfcn.h>
#include <json/json.h>
#include <cstdio>
#include <fstream>

extern "C"
{
#include <gst/check/gstcheck.h>
#include <gst/gst.h>
}
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include "GstTdkSrc.h"

using namespace std;
using namespace tinyxml2;


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
#define PROGRESS_BAR_WIDTH              50
#define CHUNK_SIZE                      4096
#define PROGRESS_BAR_DISPLAY_INTERVAL   5 //seconds 
#define resolution_count		3
#define LOG_FILE                        "/opt/TDK/mediapipeline_test_step.log"
#define DOT_GENERATE_SETUP(playbin) if(playbin) generate_dot_graph(playbin, "setup", current_test_name.c_str())
#define DOT_GENERATE_PLAYING(playbin) if(playbin) generate_dot_graph(playbin, "playing", current_test_name.c_str())
#define DOT_GENERATE_FINAL(playbin) if(playbin) generate_dot_graph(playbin, "final", current_test_name.c_str())

char m_play_url[BUFFER_SIZE_LONG] = {'\0'};
char tcname[BUFFER_SIZE_SHORT] = {'\0'};
char TDK_PATH[BUFFER_SIZE_SHORT] = {'\0'};
char channel_url[BUFFER_SIZE_LONG] = {'\0'};
char audio_change_test[BUFFER_SIZE_SHORT] = {'\0'};
vector<string> operationsList;
string current_test_name = "mediapipeline";

/*
 * Default values for avstatus check flag and play_timeout if not received as input arguments
 */

bool checkAVStatus = false;
bool Switch_up = false;
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
bool ignoreEOS = false;
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
bool checkNewPlay = true;
bool only_audio = false;
bool only_video = false;
bool checkEachSecondPlayback = false;
bool use_appsrc = false;
bool use_multiappsrc = false;
int ReadSize = 0;
int expected_height = 0;
guint64 total_bytes_fed = 0;
guint64 BYTES_THRESHOLD = 0;
float previous_progress_percentage = 0.0;
bool force_appsrc = false;
bool finish_feed = false;
bool video_underflow_test = false;
bool audio_underflow_test = false;
bool forceAudioSink = false;
bool seek_with_pause = false;
bool westerosStarted = false;
std::vector<void *> handles;
bool curlError = false;
guint bandwidth;
GstElement *dash_demux;
FILE *file = NULL;
bool log_enabled = true;
bool playbackValidationLog = false;
bool defaultStart = true;
bool enable_last_sample = false;
gint64 buffer_PTS = 0;
bool live_stream = false;
bool resolutionSeen[TOTAL_RESOLUTIONS_COUNT] = {false};
gint64 seek_position = -1;  /* Seek position for test_seek_EOS (in nanoseconds). -1 means not provided. */

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
    gboolean demuxed;
    GQueue *auQueue;
    GstCaps *caps;                       /* Demuxed stream capabilities */
    guint64 demuxed_cumulative_duration_ns;  /* Track cumulative duration for demuxed queue feeding */
    gboolean segment_event_sent;              /* Flag to track if segment event has been sent */
    GList *current_buffer_iter;               /* Current position in buffer queue for seeking/feeding */
    guint64 current_byte_offset;              /* Current byte/time offset for seeking */
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
    guint current_bandwidth;
} MessageHandlerData;

/*
 * Methods
 */
static void handleMessage (MessageHandlerData *data, GstMessage *message);

/********************************************************************************************************************
Purpose:               Generate DOT graph for GStreamer pipeline visualization
Parameters:
pipeline               - The GStreamer pipeline element
stage_name             - Stage identifier (e.g., "init", "playing", "post_seek", "final")
test_name              - Test name for filename identification
********************************************************************************************************************/
void generate_dot_graph(GstElement *pipeline, const char *stage_name, const char *test_name = NULL)
{
    if (!pipeline) {
        printf("[DOT] WARNING: Cannot generate dot graph - pipeline is NULL\n");
        return;
    }

    const char *dot_dir = g_getenv("GST_DEBUG_DUMP_DOT_DIR");
    if (!dot_dir) {
        printf("[DOT] GST_DEBUG_DUMP_DOT_DIR is not set , not proceeding to capture dot graph\n");
        return;
    }

    const char *use_test_name = test_name ? test_name : current_test_name.c_str();

    /* Append a timestamp so each capture gets a unique name and is not overwritten */
    char timestamp[32];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", tm_info);

    char filename[512];
    mkdir(dot_dir, 0755);
    snprintf(filename, sizeof(filename), "%s/MediaPipelineSuite_%s_%s_%s.dot", dot_dir, use_test_name, stage_name, timestamp);

    char just_filename[256];
    snprintf(just_filename, sizeof(just_filename), "MediaPipelineSuite_%s_%s_%s", use_test_name, stage_name, timestamp);

    GST_DEBUG_BIN_TO_DOT_FILE((GstBin *)pipeline, GST_DEBUG_GRAPH_SHOW_ALL, just_filename);
    
    if (access(filename, F_OK) == 0) {
        printf("[DOT] SUCCESS: DOT file created: %s\n", filename);
    } else {
        printf("[DOT] INFO: DOT file may be in %s\n", dot_dir);
    }
}

bool fileExists(const char* filename, bool returnResult = false)
{
   std::ifstream file_(filename);
   if (!file_.good())
   {
       if (returnResult)
       {
           printf("\n %s not present in DUT", filename);
	   return false;
       }
       if (log_enabled)
	   fprintf(file,"ERROR : File not found\n %s : No such file present in DUT", filename);
       printf("ERROR : File not found\n %s : No such file present in DUT", filename);
       fclose(file);
       fail_unless(false,"ERROR : File not found");
   }
   return true;
}

void startPlaybackValidationLogging(bool start=true)
{
   if (file)
   {
       if (start)
       {
	   fprintf(file,"\n################################\n");
           fprintf(file,"# Playback Validation Started\n");
	   fprintf(file,"################################\n");
	   playbackValidationLog = true;
	   log_enabled = true;
       }
       else
       {
	   fprintf(file,"\n################################\n");
	   fprintf(file,"# Playback Validation Ended\n");
	   fprintf(file,"################################\n");
	   playbackValidationLog = false;
       }
   }
}

void assert_failure(GstElement* playbin, bool success, const char *str= "Failure occured", const char* func= "default function", int line= 0, const char* test_step="Test Step")
{
   if (strstr(test_step, "Test Step") != nullptr)
   {
       log_enabled = false;
   }
   if (log_enabled)
   {
       fprintf(file,"TEST STEP : %s  ",test_step);
       if (!playbackValidationLog)
	    fprintf(file,"\n");
   }
   if(success)
   {
      if (log_enabled)
      {
	  if (playbackValidationLog)
	  {
	      fprintf(file," --> SUCCESS | ");
	  }
	  else
	  {
              fprintf(file,"RESULT : SUCCESS\n\n");
	  }
      }
      log_enabled = true;
      return;
   }

   if (log_enabled)
       fprintf(file,"RESULT : FAILURE\n");
   if(playbin)
   {
      gst_element_set_state (playbin, GST_STATE_NULL);
   }
   gst_object_unref (playbin);
   if (log_enabled)
   {
      fprintf(file,"\nFAILURE observed at %s : [%s %d]", func, __FILE__, line);
      fprintf(file,"\nFAILURE Reason : %s\n",str);
   }
   fclose(file);
   fail_unless(false,str);
}

void terminatePipeline(GstElement* playbin)
{
   DOT_GENERATE_FINAL(playbin);
   if (playbin)
   {
        assert_failure(playbin, gst_element_set_state (playbin, GST_STATE_NULL), "Failed to set playbin to NULL state",__FUNCTION__,__LINE__,"Set Pipeline to NULL state");
        gst_object_unref (playbin);
   }
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
   GstState cur_state __attribute__((unused));
   gfloat _play_jump = 0;
   int play_jump =0; 
   int play_jump_previous = 99;
   gint jump_buffer = 3;
   gint frame_buffer = 3;
   gint audio_frame_buffer = 3;
   gint jump_buffer_small_value = 2;
   GstStateChangeReturn state_change __attribute__((unused));
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
   data.eosDetected = FALSE;

   if (seekOperation)
   {
       startPosition = seekSeconds * GST_SECOND;
   }
   else
   {
       assert_failure (playbin,gst_element_query_position (playbin, GST_FORMAT_TIME, &startPosition), "Failed to query the current playback position");
   }
   g_object_get (playbin,"video-sink",&videoSink,NULL);

   if (use_audioSink)
   {
	 g_object_get (playbin,"audio-sink",&audioSink,NULL);
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

   bus = gst_element_get_bus (playbin);
   
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

	     assert_failure (playbin,rendered_frames != 0 , "Video rendered_frames is coming as 0", __FUNCTION__ ,__LINE__);  
            
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
		printf(" Expected Audio Sampling rate = %d",sampling_rate);
		printf(" Audio sampling rate diff = %d",audio_sampling_diff);


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
    startPlaybackValidationLogging(true);
    DOT_GENERATE_PLAYING(data->playbin);
    if (only_audio)
    {
	 checkPTS=false;
	 use_westerossink_fps=false;
    }
    if (only_video)
    {
	 use_audioSink=false;
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
    int resBffer = 2;
    GstMessage *message;

    // Resolution Switch Test Initializations
    vector<int> resList;
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
	printf("\nEntering to Pipeline Initiation\n");
        assert_failure (data->playbin,gst_element_query_position (data->playbin, GST_FORMAT_TIME, &(data->previousposition)), "Failed to query the current playback position",
			__FUNCTION__,__LINE__,"Querying Playback Position before starting Playback Validation" );
	if (log_enabled)
             fprintf(file, "\n");
     
	if (!live_stream)
	{
 	   assert_failure(data->playbin, GST_TIME_AS_SECONDS(data->previousposition) < 10, "Initial position of the pipeline is excessively high",
	                __FUNCTION__,__LINE__,"Verify Pipeline initiates properly");
	}

	if (log_enabled)
	     fprintf(file, "\n");
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
    assert_failure (data->playbin, gst_element_get_state (data->playbin, &(data->cur_state), NULL, 0) == GST_STATE_CHANGE_SUCCESS, "Failed to obtain playback state of pipeline",
		    __FUNCTION__,__LINE__,"Querying Pipeline State before starting Playback Validation");
    if (log_enabled)
	 fprintf(file, "\n");
    GstBus *bus;
    data->eosDetected = FALSE;
    bus = gst_element_get_bus (data->playbin);

    // seconds paramters for loop monitoring
    int milliSeconds = 0;
    int previous_seconds = 0;
    int second_count = 0;

    int iterator =1;

    // loop timer
    auto loopStart = std::chrono::high_resolution_clock::now();

    if (log_enabled)
	 fprintf(file,"\n");

    while(1)
    {
	 if (log_enabled)
	      fprintf(file,"Iteration %02d ", iterator);
	 /* Loop break condition   
	  * Break after executing the desired number of seconds
	  */
         if (std::chrono::high_resolution_clock::now() - loopStart > std::chrono::seconds(seconds))
	 {
	      auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - loopStart).count();

              // Print the duration using printf
              printf("\nBreaking Playback Validation loop after %02lld seconds\n", duration);
	      if (log_enabled)
		  fprintf(file,"\nBreaking Playback Validation loop after %02lld seconds\n", duration);
              break;
	 }
     
	 // Second Counter
	 second_count = milliSeconds/1000;

	 message = gst_bus_pop_filtered(bus,(GstMessageType)((GstMessageType) GST_MESSAGE_ERROR | (GstMessageType) GST_MESSAGE_EOS ));
         if (NULL != message)
         {
             handleMessage (data, message);
         }

	 if (data->eosDetected == TRUE)
         {
             if (ignoreEOS)
	     {
	          printf("Not breaking even though EOS is detected");
		  if (log_enabled)
		      fprintf(file,"\nNot breaking even though EOS is detected\n");
	     }
	     else
	     {
	          printf("\nBreaking due to EOS detected\n");
		  if (log_enabled)
		      fprintf(file,"\nBreaking due to EOS detected\n");
 	          break;
             }
	 }


	 /* Playback Position Validation
	  * By default Playback Position Validation is for each 100 milliseconds
	  * If user wants to override and make it for each second once checkEachSecondPlayback must be set to true
	  */
	 if (((checkEachSecondPlayback) && ((second_count > previous_seconds) || (milliSeconds == 0))) || (!checkEachSecondPlayback))
         {
       
              assert_failure (data->playbin,gst_element_query_position (data->playbin, GST_FORMAT_TIME, &(data->currentPosition)), "Failed to query the current playback position",
			      __FUNCTION__,__LINE__,"Query Playback Position");
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
		  {
		      char buffer[256];
		      sprintf(buffer,"Difference in expected position is large \nTDK LOG :  %02d:%02d:%02d  -- Playback position: %" GST_TIME_FORMAT ""
		                     " Expected Playback position:  %" GST_TIME_FORMAT "    Position diff: %.5f seconds",
				      timeStruct->tm_hour, timeStruct->tm_min, timeStruct->tm_sec,GST_TIME_ARGS(data->currentPosition),
				      GST_TIME_ARGS(data->previousposition),abs(position_diff_seconds));
                      assert_failure (data->playbin, false, buffer, __FUNCTION__,__LINE__,"Playback Position Validation");
		  }
		  else
		  {
		      assert_failure (data->playbin, true, NULL, __FUNCTION__,__LINE__,"Playback Position Validation");
		  }
		  /*
                  if (abs(position_diff_seconds) > 0.9)
                       gst_element_set_state (data->playbin, GST_STATE_NULL);
                  fail_unless( position_diff_seconds < 0.9,"Difference in expected position is large \nTDK LOG :  %02d:%02d:%02d  -- Playback position: %" GST_TIME_FORMAT " Expected Playback position:  %" GST_TIME_FORMAT "    Position diff: %.5f seconds",timeStruct->tm_hour, timeStruct->tm_min, timeStruct->tm_sec,GST_TIME_ARGS(data->currentPosition),GST_TIME_ARGS(data->previousposition),abs(position_diff_seconds));
		  */
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
                 assert_failure (data->playbin,data->westerosSink.pts_buffer != 0 , "Video is not playing according to video-data->westerosSink.pts check of westerosSink",
				 __FUNCTION__,__LINE__,"Video PTS Validation");
             }
             data->westerosSink.old_pts = data->westerosSink.pts;
         }

	 /* Video Frames validation along with seconds counter
	  * Video validation is done for each second instead of 100 milliseconds
	  */
	 if ((use_westerossink_fps) && (second_count > previous_seconds))
         {
	     fprintf(file,"\n");
             g_object_get (data->westerosSink.sink,"stats",&(data->westerosSink.structure),NULL);
	     gst_structure_get_uint64(data->westerosSink.structure, "rendered", &(data->westerosSink.rendered_frames));
	     if (data->westerosSink.structure && (gst_structure_has_field(data->westerosSink.structure, "dropped") || gst_structure_has_field(data->westerosSink.structure, "rendered")))
	     {
		 gst_structure_get_uint64(data->westerosSink.structure, "dropped", &(data->westerosSink.dropped_frames));
                 gst_structure_get_uint64(data->westerosSink.structure, "rendered", &(data->westerosSink.rendered_frames));
		 printf("\n\nVideo Frames :");
                 printf(" Dropped: %" G_GUINT64_FORMAT, data->westerosSink.dropped_frames);
		 printf(" Rendered: %" G_GUINT64_FORMAT , data->westerosSink.rendered_frames);

		 assert_failure (data->playbin,data->westerosSink.rendered_frames != 0 , "Video rendered_frames is coming as 0",
				 __FUNCTION__,__LINE__,"Verify Video Frame is Rendered");
		 if (((data->westerosSink.rendered_frames <= data->westerosSink.previous_rendered_frames)) &&  (data->cur_state == GST_STATE_PLAYING))
                    data->westerosSink.frame_buffer -= 1;
		 else if (((data->westerosSink.rendered_frames != data->westerosSink.previous_rendered_frames)) &&  (data->cur_state == GST_STATE_PAUSED))
		    data->westerosSink.frame_buffer -= 1;

		 assert_failure (data->playbin,data->westerosSink.frame_buffer != 0 , "Video frames are not rendered properly",
				 __FUNCTION__,__LINE__,"Verify Video Frames rendered are incrementing");
		 data->westerosSink.previous_rendered_frames = data->westerosSink.rendered_frames;
	     }

	 }

	 /* Audio Frames validation along with seconds counter
	  * Audio validation is done for each second instead of 100 milliseconds
	  */
         if ((use_audioSink) && (second_count > previous_seconds))
         {
	     fprintf(file,"\n");
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

		 //assert_failure (data->playbin, data->audioSink.rendered_frames != 0 , "Audio rendered_frames is coming as 0",
                                 //__FUNCTION__,__LINE__,"Verify Audio Frame is Rendered");
                 if ((data->audioSink.rendered_frames <= data->audioSink.previous_rendered_frames) &&  (data->cur_state == GST_STATE_PLAYING))
                    data->audioSink.frame_buffer -= 1;
		 else if ((data->audioSink.rendered_frames != data->audioSink.previous_rendered_frames) &&  (data->cur_state == GST_STATE_PAUSED))
		    data->audioSink.frame_buffer -= 1;

                 assert_failure (data->playbin,data->audioSink.frame_buffer != 0 , "Audio frames are not rendered properly",
				 __FUNCTION__,__LINE__,"Verify Audio Frames rendered are incrementing");
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
		 assert_failure (data->playbin,audio_sampling_rate_buffer != 0, "Audio sampling rate was not rendered properly",
				 __FUNCTION__,__LINE__,"Verify Audio Sampling rate");

		 if (sampling_rate == 96)
                     rate_diff_threshold = -5;
	     }
	     data->audioSink.previous_rendered_frames =  data->audioSink.rendered_frames;

         }

	 // Video underflow break condition
	 if(videoUnderflowReceived && bufferUnderflowTest)
         {
             printf("\nVideo Underflow received breaking from PlaybackValidation");
	     if (log_enabled)
		 fprintf(file,"\nVideo Underflow received breaking from PlaybackValidation\n");
             break;
         }

	 // Audio underflow break condition
	 if(audio_underflow_received_global && bufferUnderflowTest)
         {
             printf("\nAudio Underflow received breaking from PlaybackValidation");
	     if (log_enabled)
		 fprintf(file,"\nAudio Underflow received breaking from PlaybackValidation\n");
             break;
         }

	 if (ResolutionSwitchTest)
	 {
    		int new_height, new_width;
		g_object_get(data->westerosSink.sink, "video-height", &new_height, NULL);
    		g_object_get(data->westerosSink.sink, "video-width", &new_width, NULL);
		if ((abs(new_height - resList[0]) < RESOLUTION_OFFSET) && !resolutionSeen[0])
		{
			resolutionSeen[0] = true;
                        ResolutionCount = 1;
		}
    		if (new_height != height)
    		{
        		height = new_height;
        		// Loop through all known resolutions
		        for (int i = 1; i < TOTAL_RESOLUTIONS_COUNT; i++)
        		{
            			if (!resolutionSeen[i] && abs(new_height - resList[i]) < RESOLUTION_OFFSET)
            			{
                			resolutionSeen[i] = true;
                			ResolutionCount++;
                			printf("\nNew unique resolution: %d (Matched with resList[%d])\n", new_height, i);
                			printf("ResolutionCount updated to %d\n", ResolutionCount);
               				break;
            			}
        		}
        		if ((new_height != data->westerosSink.height) || (new_width != data->westerosSink.width))
        		{
            			data->westerosSink.height = new_height;
            			data->westerosSink.width = new_width;
            			printf("\nVideo height = %d\nVideo width = %d", data->westerosSink.height, data->westerosSink.width);
            			// Optional: Check if this resolution was expected
            			bool matched = false;
            			for (int i = 0; i < TOTAL_RESOLUTIONS_COUNT; i++)
            			{
                			if (abs(data->westerosSink.height - resList[i]) < RESOLUTION_OFFSET)
                			{
                    				matched = true;
                   				break;
                			}
            			}
            			if (log_enabled)
                			fprintf(file, "\n");
            			if (!matched)
            			{
                			resBffer -= 1;
            			}
            			else
            			{
                			resBffer = 2;
            			}
        		}
        		if (ResolutionCount == TOTAL_RESOLUTIONS_COUNT)
        		{
            			printf("\nPipeline played all unique resolutions\n");
            			break;
        		}
    		}
	}


         // Seconds Counter
	 if (second_count > previous_seconds)
	 {
	     previous_seconds += 1;
	     fprintf(file, "\n");
	 }

         // Sleeping for  100 milliseconds
	 MilliSleep(100);
	 milliSeconds += 100;

	 iterator++;
	 fprintf(file, "\n");
     }

     printf("\nExiting from PlaybackValidation, currentPosition is %lld\n",(data->currentPosition)/GST_SECOND);
     if (log_enabled)
         fprintf(file, "\nExiting from PlaybackValidation, currentPosition is %lld\n",(data->currentPosition)/GST_SECOND);
     startPlaybackValidationLogging(false);
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
	if (getenv ("TDK_NATIVE_AUDIO") != NULL)
	{
	    flags |= GST_PLAY_FLAG_NATIVE_AUDIO;
	    if (log_enabled)
		fprintf(file,"\nEnabled NATIVE_AUDIO flag for playbin\n");
	    printf("\nEnabled NATIVE_AUDIO flag for playbin\n");
        }
	if (getenv ("TDK_NATIVE_VIDEO") != NULL)
        {
            flags |= GST_PLAY_FLAG_NATIVE_VIDEO;
	    if (log_enabled)
                fprintf(file,"\nEnabled NATIVE_VIDEO flag for playbin\n");
            printf("\nEnabled NATIVE_VIDEO flag for playbin\n");
        }
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
	    printf("Error in opening pipe \n");
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
    printf("Script Output: %s %s\n", script, result);
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
    if (g_str_has_prefix(GST_ELEMENT_NAME(element), "dashdemux"))
    {
	g_print ("\nGot Dashdemux\n");
        dash_demux = element;
	if ( data->current_bandwidth > 0 )
            g_object_set(G_OBJECT(element), "max-bitrate", data->current_bandwidth, NULL);
    }



}


// Helper function to write downloaded data into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) 
{
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}



/******************************************************************************************************************
 * Purpose:            Funtion to parse the manifest file
 *****************************************************************************************************************/


bool parse_mpd(const string& m_play_url, vector<pair<unsigned int, unsigned int>>& resolutions)
{
    // Create a TinyXML-2 XMLDocument object
    XMLDocument doc;
    string mpdFile = m_play_url;
    XMLError eResult;

    // Check and remove "file:" prefix if present in the URL
    const string file_prefix = "file:";
    bool is_local_file = true;

    if (mpdFile.find(file_prefix) == 0)
    {
        mpdFile = mpdFile.substr(file_prefix.length());
    }
    else if (mpdFile.find("http://") == 0 || mpdFile.find("https://") == 0)
    {
        // It's an online stream, so download the MPD file
        is_local_file = false;
        CURL* curl;
        CURLcode res;
        std::string mpd_content;
        curl = curl_easy_init();
        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, mpdFile.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mpd_content);

            // Perform the request, res will get the return code
            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                cerr << "Failed to download MPD file: " << curl_easy_strerror(res) << endl;

                curl_easy_cleanup(curl);
                return false;
            }
            curl_easy_cleanup(curl);

            // Parse the downloaded MPD content
            eResult = doc.Parse(mpd_content.c_str());
        }
    }

    // Load the MPD XML file if it's a local file
    if (is_local_file)
    {
        eResult = doc.LoadFile(mpdFile.c_str());
    }

    if (eResult != XML_SUCCESS)
    {
        cerr << "Failed to load MPD file: " << doc.ErrorName() << endl;
        return false;
    }

    // Get the root element (MPD)
    XMLElement* mpd = doc.FirstChildElement("MPD");
    if (mpd == nullptr)
    {
        cerr << "No MPD element found." << endl;
        return false;
    }

    // Iterate over Period elements
    XMLElement* period = mpd->FirstChildElement("Period");
    while (period != nullptr)
    {
        // Iterate over AdaptationSet elements
        XMLElement* adaptationSet = period->FirstChildElement("AdaptationSet");
        while (adaptationSet != nullptr)
        {
            const char* contentType = adaptationSet->Attribute("contentType");
            if ((contentType && strcmp(contentType, "video") == 0) || ( ! contentType))
            { 
                XMLElement* representation = adaptationSet->FirstChildElement("Representation");
                while (representation != nullptr)
                {
		    const char* mimeType = representation->Attribute("mimeType");
		    if ((mimeType && strcmp(mimeType, "video/mp4") == 0) || ( ! mimeType))
		    {
                    	const char* height_str = representation->Attribute("height");
                    	const char* bandwidth_str = representation->Attribute("bandwidth");

                    	if (height_str && bandwidth_str)
                    	{
                        	unsigned int height = (unsigned int)atoi(height_str);
	                        unsigned int bandwidth = (unsigned int)atoi(bandwidth_str);
	
        	                // Push height and bandwidth into the resolutions vector
                	        resolutions.push_back(make_pair(height, bandwidth));

                        	// Debugging print
                        	printf("Parsed Height: %u, Bandwidth: %u\n", height, bandwidth);
                    	}
		    }
                    representation = representation->NextSiblingElement("Representation");
		}
	    }
            
            adaptationSet = adaptationSet->NextSiblingElement("AdaptationSet");
        }
        period = period->NextSiblingElement("Period");
    }
    return true;
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
    if (log_enabled)
	fprintf(file, "\nFlushing pipeline\n");    
    printf("\nFlushing Pipeline after switch\n");
    assert_failure (data->playbin,gst_element_query_position (data->playbin, GST_FORMAT_TIME, &(data->currentPosition)),
                                         "Failed to query the current playback position");
    seekSeconds = (data->currentPosition)/GST_SECOND;
    assert_failure (data->playbin,gst_element_seek (data->playbin, NORMAL_PLAYBACK_RATE, GST_FORMAT_TIME,
                                   GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, data->currentPosition,
                                   GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE), "Failed to seek");
    Sleep(2);
    audio_underflow_received = false;
    assert_failure (data->playbin,gst_element_query_position (data->playbin, GST_FORMAT_TIME, &(data->previousposition)), "Failed to query the current playback position",
		    __FUNCTION__,__LINE__, "Query playback position after flush");
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

  if (log_enabled)
  {
      fprintf(file, "Previous volume : %.0f%% \n", volume * 100);
      fprintf(file, "Setting volume to %f \n", volume_set);
  }      
  
  set_volume = volume_set;

  printf ("Volume setted : %.0f%% \n", set_volume * 100);
  if (log_enabled)
      fprintf(file, "Volume setted : %.0f%% \n", set_volume * 100);

  gst_stream_volume_set_volume (GST_STREAM_VOLUME (playbin),
      GST_STREAM_VOLUME_FORMAT_CUBIC, set_volume);

  get_volume = gst_stream_volume_get_volume (GST_STREAM_VOLUME (playbin),
      GST_STREAM_VOLUME_FORMAT_CUBIC);
 
  printf ("Volume: %.6f%% \n", get_volume * 100);
  if (log_enabled)
       fprintf(file, "Volume: %.6f%% \n", get_volume * 100);

  GstElement* audio_sink = NULL;
  g_object_get(playbin, "audio-sink", &audio_sink, NULL);
  if (audio_sink)
  {
       if (g_object_class_find_property(G_OBJECT_GET_CLASS(audio_sink), "volume"))
       {
           double sink_volume;
           /* Get volume from sink and also get it from playbin in LINEAR format for proper comparison */
           g_object_get(G_OBJECT(audio_sink), "volume", &sink_volume, NULL);
           double linear_volume = gst_stream_volume_get_volume(GST_STREAM_VOLUME(playbin),
                                                                GST_STREAM_VOLUME_FORMAT_LINEAR);
           
	   printf("\n\nVolume from audiosink = %.6f\n", sink_volume);
	   printf("Expected volume (linear) = %.6f\n\n", linear_volume);

	   /* Compare sink volume (linear) with playbin volume in linear format */
	   assert_failure (playbin, fabs(sink_volume - linear_volume) < 0.01, "audio-sink doesn't return the expected volume", __FUNCTION__, __LINE__, "Verify if volume is properly set in audio-sink");
       }
       g_object_unref(audio_sink);
  }

  assert_failure (playbin, fabs(set_volume - get_volume) < 0.01, "Failed to set the given volume", __FUNCTION__, __LINE__, "Verify if volume is set successfully");
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

    if (data->app.demuxed)
    {
        if (data->app.auQueue == NULL || g_queue_is_empty(data->app.auQueue))
        {
            gst_app_src_end_of_stream(GST_APP_SRC(data->app.appsrc));
            displayProgressBar("Appsrc Data Push ", 100);
            return FALSE;
        }

        /* Apply buffer limiting: push only (play_timeout + 30) seconds for generic tests
         * BUT: For channel change tests, limit to just 10 seconds per channel (faster for 4K)
         */
        if (data->app.demuxed_cumulative_duration_ns > 0)
        {
            guint64 target_duration_ns;
            if (bufferUnderflowTest && data->terminate)
            {
                /* Underflow tests phase 2: push additional 10 seconds then EOS */
                target_duration_ns = 10 * GST_SECOND;
            }
            else if (bufferUnderflowTest)
            {
                /* Underflow tests phase 1: 10 seconds to trigger underflow */
                target_duration_ns = 10 * GST_SECOND;
            }
            else
            {
                /* Generic tests: play_timeout + 30 seconds buffer */
                target_duration_ns = (play_timeout + 30) * GST_SECOND;
            }
            
            if (data->app.demuxed_cumulative_duration_ns >= target_duration_ns)
            {
                printf("\nReached target duration %" G_GUINT64_FORMAT "ns (~%u seconds) (phase %d)\n", 
                       data->app.demuxed_cumulative_duration_ns, (guint)(data->app.demuxed_cumulative_duration_ns / GST_SECOND),
                       data->terminate ? 2 : 1);
                if (log_enabled)
                    fprintf(file, "\nReached target duration %" G_GUINT64_FORMAT "ns (~%u seconds) (phase %d)\n", 
                           data->app.demuxed_cumulative_duration_ns, (guint)(data->app.demuxed_cumulative_duration_ns / GST_SECOND),
                           data->terminate ? 2 : 1);
                
                /* Emit EOS in phase 1 (after initial 40s preload) for seek tests */
                if (!data->terminate)
                {
                    printf("\n[tdksrc] Phase 1 complete: Emitting EOS after pushing %u seconds\n", (guint)(data->app.demuxed_cumulative_duration_ns / GST_SECOND));
                    if (log_enabled)
                        fprintf(file, "\n[tdksrc] Phase 1 complete: Emitting EOS after pushing %u seconds\n", (guint)(data->app.demuxed_cumulative_duration_ns / GST_SECOND));
                    gst_app_src_end_of_stream(GST_APP_SRC(data->app.appsrc));
                }
                /* Emit EOS in phase 2 when reaching additional 10 seconds */
                else if (data->terminate)
                {
                    printf("\n[tdksrc] Phase 2 complete: Emitting EOS after pushing additional 10 seconds\n");
                    if (log_enabled)
                        fprintf(file, "\n[tdksrc] Phase 2 complete: Emitting EOS after pushing additional 10 seconds\n");
                    gst_app_src_end_of_stream(GST_APP_SRC(data->app.appsrc));
                }
                return FALSE;
            }
        }

        buffer = (GstBuffer *) g_queue_pop_head(data->app.auQueue);
        guint64 buffer_duration = GST_BUFFER_DURATION(buffer);
        if (buffer_duration != GST_CLOCK_TIME_NONE)
            data->app.demuxed_cumulative_duration_ns += buffer_duration;
        
        total_bytes_fed += gst_buffer_get_size(buffer);
        g_signal_emit_by_name(data->app.appsrc, "push-buffer", buffer, &ret);
        gst_buffer_unref(buffer);
        if (ret != GST_FLOW_OK)
            return FALSE;
        return TRUE;
    }

  if ((bufferUnderflowTest) && (total_bytes_fed >= BYTES_THRESHOLD))
  {
     printf("\n reached BYTES_THRESHOLD %lld \n",BYTES_THRESHOLD);
     if (log_enabled)
	 fprintf(file, "\n reached BYTES_THRESHOLD %lld \n",BYTES_THRESHOLD);
     printf("\n data->terminate = %d\n",data->terminate);
     if (data->terminate)
     {
	 printf ("\nEmitting EOS");
	 if (log_enabled)
             fprintf(file, "\n Emitting EOS\n");
	 gst_app_src_end_of_stream(GST_APP_SRC(data->app.appsrc));
     }
     return FALSE;
  }

  if ((data->app.offset >= data->app.length)) {
    /* we are EOS, send end-of-stream and remove the source */
    gst_app_src_end_of_stream(GST_APP_SRC(data->app.appsrc));
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
/*************************************************** tdksrc reading start *****************************************/

typedef struct _TdkFeedData {
    MessageHandlerData video;
    MessageHandlerData audio;
    GstCaps *videoCaps;
    GstCaps *audioCaps;
} TdkFeedData;

static gboolean tdkIsContainer(const char *fileUrl)
{
    const char *filePrefix = "file:";
    const char *filePath = fileUrl;

    if (fileUrl == NULL)
        return FALSE;
    if (strncmp(fileUrl, filePrefix, strlen(filePrefix)) == 0)
        filePath = fileUrl + strlen(filePrefix);

    const char *ext = strrchr(filePath, '.');
    if (ext == NULL)
        return FALSE;

    return (g_ascii_strcasecmp(ext, ".mp4") == 0 ||
            g_ascii_strcasecmp(ext, ".mov") == 0 ||
            g_ascii_strcasecmp(ext, ".m4v") == 0 ||
            g_ascii_strcasecmp(ext, ".m4a") == 0 ||
            g_ascii_strcasecmp(ext, ".mkv") == 0 ||
            g_ascii_strcasecmp(ext, ".mka") == 0 ||
            g_ascii_strcasecmp(ext, ".webm") == 0 ||
            g_ascii_strcasecmp(ext, ".webmv") == 0 ||
            g_ascii_strcasecmp(ext, ".webma") == 0);
}

/* Check if URL points to an audio-only file (MP3, WAV, FLAC, etc.) */
static gboolean tdkIsAudioOnly(const char *fileUrl)
{
    const char *filePrefix = "file:";
    const char *filePath = fileUrl;

    if (fileUrl == NULL)
        return FALSE;
    if (strncmp(fileUrl, filePrefix, strlen(filePrefix)) == 0)
        filePath = fileUrl + strlen(filePrefix);

    const char *ext = strrchr(filePath, '.');
    if (ext == NULL)
        return FALSE;

    return (g_ascii_strcasecmp(ext, ".mp3") == 0 ||
            g_ascii_strcasecmp(ext, ".wav") == 0 ||
            g_ascii_strcasecmp(ext, ".flac") == 0 ||
            g_ascii_strcasecmp(ext, ".aac") == 0 ||
            g_ascii_strcasecmp(ext, ".ogg") == 0);
}

static gboolean tdkIsMp3(const char *fileUrl)
{
    const char *filePrefix = "file:";
    const char *filePath = fileUrl;

    if (fileUrl == NULL)
        return FALSE;
    if (strncmp(fileUrl, filePrefix, strlen(filePrefix)) == 0)
        filePath = fileUrl + strlen(filePrefix);

    const char *ext = strrchr(filePath, '.');
    if (ext == NULL)
        return FALSE;

    return (g_ascii_strcasecmp(ext, ".mp3") == 0);
}

/* Forward declaration for tdkDrainAppsink used by tdkDemuxMp3WithES */
static guint tdkDrainAppsink(GstElement *appsink, GQueue *queue, GstCaps **outCaps);

/* Forward declaration for tdkNeedData callback */
extern "C" {
static void tdkNeedData(GstAppSrc *appsrc, guint length, App *app);
static gboolean tdkSeekData(GstAppSrc *appsrc, guint64 offset, App *app);
} /* extern "C" */

static gboolean tdkDemuxMp3WithES(const char *fileUrl, TdkFeedData *feed)
{
    const char *filePrefix = "file:";
    const char *filePath = fileUrl;
    if (strncmp(fileUrl, filePrefix, strlen(filePrefix)) == 0)
        filePath = fileUrl + strlen(filePrefix);

    printf("\n[tdksrc] demux: MP3 file detected, feeding raw MP3 bytes for pipeline processing\n");

    /* Open MP3 file and read into buffers */
    FILE *fp = fopen(filePath, "rb");
    if (!fp)
    {
        printf("\n[tdksrc] demux: failed to open MP3 file: %s\n", filePath);
        return FALSE;
    }

    /* Get file size */
    fseek(fp, 0, SEEK_END);
    gsize file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    printf("\n[tdksrc] demux: MP3 file size: %zu bytes\n", file_size);

    /* Read file in 64KB chunks and push to queue */
    const gsize chunk_size = 65536;
    guint buffer_count = 0;
    gsize total_read = 0;

    while (total_read < file_size)
    {
        gsize to_read = (file_size - total_read > chunk_size) ? chunk_size : (file_size - total_read);
        guint8 *data = (guint8*)g_malloc(to_read);

        if (fread(data, 1, to_read, fp) != to_read)
        {
            printf("\n[tdksrc] demux: file read error at offset %zu\n", total_read);
            g_free(data);
            fclose(fp);
            return FALSE;
        }

        /* Create GStreamer buffer from file chunk */
        GstBuffer *buf = gst_buffer_new_wrapped(data, to_read);
        if (!buf)
        {
            printf("\n[tdksrc] demux: failed to create buffer\n");
            g_free(data);
            fclose(fp);
            return FALSE;
        }

        /* Set timestamp for proper synchronization */
        GST_BUFFER_PTS(buf) = GST_CLOCK_TIME_NONE;
        GST_BUFFER_DTS(buf) = GST_CLOCK_TIME_NONE;

        g_queue_push_tail(feed->audio.app.auQueue, buf);
        total_read += to_read;
        buffer_count++;
    }

    fclose(fp);

    printf("\n[tdksrc] demux: loaded %u MP3 buffer chunks (%.2f MB total)\n", 
           buffer_count, (float)total_read / (1024.0 * 1024.0));

    /* MP3 raw bytes are valid MPEG audio; set the caps explicitly so appsrc can negotiate
     * correctly with the downstream parse/decoder chain. */
    feed->audioCaps = gst_caps_new_simple("audio/mpeg",
        "mpegversion", G_TYPE_INT, 1,
        "layer", G_TYPE_INT, 3,
        "rate", G_TYPE_INT, 44100,
        "channels", G_TYPE_INT, 2,
        "parsed", G_TYPE_BOOLEAN, TRUE,
        NULL);

    feed->video.app.demuxed = FALSE;
    feed->audio.app.demuxed = (buffer_count > 0);
    return (buffer_count > 0);
}

typedef struct _TdkDemuxSinks {
    GstElement *videoSink;
    GstElement *audioSink;
} TdkDemuxSinks;

static void tdkDemuxPadAdded(GstElement *, GstPad *pad, gpointer user)
{
    TdkDemuxSinks *sinks = (TdkDemuxSinks *) user;
    GstCaps *caps = gst_pad_get_current_caps(pad);
    if (caps == NULL)
        caps = gst_pad_query_caps(pad, NULL);

    const gchar *name = gst_structure_get_name(gst_caps_get_structure(caps, 0));
    GstElement *sink = NULL;
    if (g_str_has_prefix(name, "video/"))
        sink = sinks->videoSink;
    else if (g_str_has_prefix(name, "audio/"))
        sink = sinks->audioSink;

    if (sink != NULL)
    {
        GstPad *sinkpad = gst_element_get_static_pad(sink, "sink");
        if (!gst_pad_is_linked(sinkpad))
        {
            if (gst_pad_link(pad, sinkpad) != GST_PAD_LINK_OK)
                printf("\n[tdksrc] demux: failed to link %s pad to appsink\n", name);
            else
                printf("\n[tdksrc] demux: linked %s pad to appsink\n", name);
        }
        gst_object_unref(sinkpad);
    }

    gst_caps_unref(caps);
}

/* Need-data callback: feeds buffers on-demand during playback
 * Pushes ~500ms of buffers per callback to keep pipeline supplied with data */
extern "C" {
static void tdkNeedData(GstAppSrc *appsrc, guint length, App *app)
{
    if (app == NULL || app->appsrc == NULL || app->auQueue == NULL)
        return;
    
    GstFlowReturn ret;
    GstBuffer *buffer;
    
    /* Handle sticky events on first need-data call */
    if (!app->segment_event_sent && app->appsrc != NULL)
    {
        GstPad *srcpad = gst_element_get_static_pad(app->appsrc, "src");
        if (srcpad)
        {
            /* Send stream-start event first */
            GstEvent *stream_start_event = gst_event_new_stream_start("tdksrc-stream");
            if (!gst_pad_push_event(srcpad, stream_start_event))
                printf("[tdksrc] Warning: Failed to push stream-start event\n");
            
            /* Send caps event second */
            if (app->caps != NULL)
            {
                GstEvent *caps_event = gst_event_new_caps(app->caps);
                if (!gst_pad_push_event(srcpad, caps_event))
                    printf("[tdksrc] Warning: Failed to push caps event\n");
            }
            
            /* Send segment event with current playback position */
            GstSegment segment;
            gst_segment_init(&segment, GST_FORMAT_TIME);
            segment.start = 0;
            segment.position = 0;
            GstEvent *segment_event = gst_event_new_segment(&segment);
            if (!gst_pad_push_event(srcpad, segment_event))
                printf("[tdksrc] Warning: Failed to push segment event\n");
            
            gst_object_unref(srcpad);
        }
        app->segment_event_sent = TRUE;
    }
    
    /* Feed multiple buffers per callback to prevent starvation during continuous playback.
       Push buffers until we reach a time threshold (500ms) or run out of buffers.
       This ensures the pipeline has enough buffered data ahead of playback position. */
    if (app->current_buffer_iter != NULL)
    {
        guint64 accumulated_time_ns = 0;
        guint64 buffer_threshold_ns = 500 * GST_MSECOND;  /* Push ~500ms of buffers per callback */
        guint buffers_pushed_this_call = 0;
        
        while (app->current_buffer_iter != NULL && accumulated_time_ns < buffer_threshold_ns)
        {
            /* Extra safety check: validate iterator and buffer data */
            if (app->current_buffer_iter->data == NULL)
            {
                printf("[tdksrc] tdkNeedData: Skipping NULL buffer in iterator\n");
                app->current_buffer_iter = app->current_buffer_iter->next;
                continue;
            }
            
            buffer = (GstBuffer *)app->current_buffer_iter->data;
            if (buffer != NULL && GST_IS_BUFFER(buffer))
            {
                guint64 buffer_duration = GST_BUFFER_DURATION(buffer);
                if (buffer_duration != GST_CLOCK_TIME_NONE)
                    app->demuxed_cumulative_duration_ns += buffer_duration;
                
                guint buffer_size = gst_buffer_get_size(buffer);
                if (buffer_size > 0)
                    total_bytes_fed += buffer_size;
                
                /* Push the buffer using direct API.
                 * Take an extra reference to protect the buffer across multiple
                 * feeding cycles and seek operations. This ensures the buffer
                 * remains valid in the queue even after the pipeline processes it. */
                gst_buffer_ref(buffer);
                ret = gst_app_src_push_buffer(GST_APP_SRC_CAST(appsrc), buffer);
                
                if (ret == GST_FLOW_OK)
                {
                    buffers_pushed_this_call++;
                    if (buffer_duration != GST_CLOCK_TIME_NONE)
                        accumulated_time_ns += buffer_duration;
                }
                else
                {
                    /* Push failed, stop feeding buffers this cycle */
                    app->current_buffer_iter = app->current_buffer_iter->next;
                    break;
                }
            }
            else
            {
                printf("[tdksrc] tdkNeedData: Invalid buffer pointer, skipping\n");
            }
            
            /* Move iterator to next buffer */
            app->current_buffer_iter = app->current_buffer_iter->next;
        }
        
        /* Check if queue is now exhausted (iterator became NULL) */
        if (app->current_buffer_iter == NULL && ret == GST_FLOW_OK)
        {
            /* All buffers have been pushed. Emit EOS. */
            g_signal_emit_by_name(app->appsrc, "end-of-stream", &ret);
        }
    }
    else
    {
        /* Iterator is NULL - either pre-load completed or all buffers consumed.
           Emit EOS to signal end-of-stream. */
        GstFlowReturn eos_ret = GST_FLOW_OK;
        g_signal_emit_by_name(appsrc, "end-of-stream", &eos_ret);
    }
}
} /* extern "C" - close tdkNeedData */

/* Seek-data callback: repositions buffer queue to requested time offset
 * Note: With format=GST_FORMAT_TIME, offset is in nanoseconds (not bytes).
 * This callback uses buffer timestamps to find the correct seek position. */
extern "C" {
static gboolean tdkSeekData(GstAppSrc *appsrc, guint64 offset, App *app)
{
    if (app == NULL || app->auQueue == NULL)
    {
        printf("\n[tdksrc] seek-data: invalid app or queue\n");
        return FALSE;
    }

    /* offset is in nanoseconds for time-based seeking */
    printf("\n[tdksrc] seek-data: seeking to time %" GST_TIME_FORMAT " ns\n", GST_TIME_ARGS(offset));

    GList *iter = app->auQueue->head;
    GstBuffer *first_buf = NULL;
    GList *last_iter = NULL;

    /* Find first buffer with valid PTS (skip buffers with invalid/corrupted timestamps) */
    while (iter != NULL && first_buf == NULL)
    {
        if (iter->data != NULL)
        {
            GstBuffer *buf = (GstBuffer *)iter->data;
            if (GST_IS_BUFFER(buf))
            {
                guint64 buf_pts = GST_BUFFER_PTS(buf);
                /* Accept only valid PTS values (not NONE, not unreasonably large) */
                if (buf_pts != GST_CLOCK_TIME_NONE && buf_pts < (guint64)(200 * GST_SECOND))
                {
                    first_buf = buf;
                    break;
                }
            }
        }
        iter = iter->next;
    }
    
    last_iter = g_queue_peek_tail_link(app->auQueue);

    /* Check for BACKWARD seek to time BEFORE the queue starts */
    if (first_buf != NULL)
    {
        guint64 first_buf_pts = GST_BUFFER_PTS(first_buf);
        if (first_buf_pts != GST_CLOCK_TIME_NONE && offset < first_buf_pts)
        {
            /* Seeking backward before queue start (e.g., seek to 0 from position 10s) - reposition to HEAD */
            /* Validate that HEAD contains a valid buffer before repositioning */
            if (app->auQueue->head != NULL && app->auQueue->head->data != NULL)
            {
                app->current_buffer_iter = app->auQueue->head;
                app->current_byte_offset = offset;
                printf("[tdksrc] seek-data: BACKWARD seek before queue, repositioned to HEAD (first buffer at %" GST_TIME_FORMAT ")\n",
                       GST_TIME_ARGS(first_buf_pts));
                return TRUE;
            }
            else
            {
                printf("[tdksrc] seek-data: BACKWARD seek but queue HEAD is invalid, staying at current position\n");
                return FALSE;
            }
        }
    }

    /* Traverse queue to find buffer at or after requested time */
    while (iter != NULL)
    {
        /* Skip NULL buffer entries */
        if (iter->data == NULL)
        {
            iter = iter->next;
            continue;
        }
        
        GstBuffer *buf = (GstBuffer *)iter->data;
        if (!GST_IS_BUFFER(buf))
        {
            iter = iter->next;
            continue;
        }
        
        guint64 buf_pts = GST_BUFFER_PTS(buf);
        guint64 buf_duration = GST_BUFFER_DURATION(buf);

        /* Check if this buffer contains the seek time */
        if (buf_pts != GST_CLOCK_TIME_NONE && buf_duration != GST_CLOCK_TIME_NONE)
        {
            if (buf_pts <= offset && offset < buf_pts + buf_duration)
            {
                /* Found the buffer at the seek position */
                app->current_buffer_iter = iter;
                app->current_byte_offset = offset;  /* Store time offset */
                printf("[tdksrc] seek-data: repositioned to buffer at time %" GST_TIME_FORMAT "\n", GST_TIME_ARGS(buf_pts));
                return TRUE;
            }
        }

        iter = iter->next;
    }

    /* If we get here, seek is beyond current buffers OR no exact match found.
       Try to reposition to the closest valid buffer or last valid buffer in queue */
    
    /* First, try to find the last valid buffer in the queue by traversing from tail backwards */
    GList *last_valid_iter = NULL;
    GList *check_iter = app->auQueue->head;
    while (check_iter != NULL)
    {
        if (check_iter->data != NULL)
        {
            GstBuffer *buf = (GstBuffer *)check_iter->data;
            if (GST_IS_BUFFER(buf))
            {
                guint64 buf_pts = GST_BUFFER_PTS(buf);
                if (buf_pts != GST_CLOCK_TIME_NONE && buf_pts < (guint64)(200 * GST_SECOND))
                {
                    last_valid_iter = check_iter;  /* Keep track of last valid buffer found */
                }
            }
        }
        check_iter = check_iter->next;
    }
    
    /* If we found a valid buffer, use it */
    if (last_valid_iter != NULL)
    {
        app->current_buffer_iter = last_valid_iter;
        GstBuffer *last_valid_buf = (GstBuffer *)last_valid_iter->data;
        printf("[tdksrc] seek-data: FORWARD seek beyond queue, repositioned to last valid buffer at %" GST_TIME_FORMAT "\n",
               GST_TIME_ARGS(GST_BUFFER_PTS(last_valid_buf)));
        return TRUE;
    }
    
    /* No valid buffers found anywhere in queue - emit EOS immediately */
    printf("[tdksrc] seek-data: No valid buffers in queue for seek to %" GST_TIME_FORMAT ", emitting EOS\n", GST_TIME_ARGS(offset));
    if (app->appsrc != NULL)
    {
        GstFlowReturn ret = GST_FLOW_OK;
        g_signal_emit_by_name(app->appsrc, "end-of-stream", &ret);
    }
    return TRUE;  /* Return TRUE to accept the seek, we'll send EOS instead */

    app->current_byte_offset = offset;
    return TRUE;
}
} /* extern "C" */

static guint tdkDrainAppsink(GstElement *appsink, GQueue *queue, GstCaps **outCaps)
{
    guint count = 0;
    GstSample *sample;
    
    /* Quick check: try to pull sample with short timeout (100ms)
     * If no caps and no samples, this appsink was never used (e.g., no video in audio-only file)
     * Skip the 5-second wait in that case
     */
    guint64 short_timeout_ns = 100 * GST_MSECOND;  /* 100ms for quick check */
    guint64 long_timeout_ns = 5 * GST_SECOND;      /* 5s for active streams */
    
    sample = gst_app_sink_try_pull_sample(GST_APP_SINK(appsink), short_timeout_ns);
    if (sample == NULL && *outCaps == NULL)
    {
        /* No sample and no caps = unused appsink, skip the long drain */
        return 0;
    }
    
    /* Process the sample we got (if any) */
    if (sample != NULL)
    {
        if (*outCaps == NULL)
        {
            GstCaps *c = gst_sample_get_caps(sample);
            if (c != NULL)
                *outCaps = gst_caps_copy(c);
        }

        GstBuffer *buf = gst_sample_get_buffer(sample);
        if (buf != NULL)
        {
            g_queue_push_tail(queue, gst_buffer_ref(buf));
            count++;
        }
        gst_sample_unref(sample);
    }
    
    /* Now do the full drain with longer timeout for active streams */
    while ((sample = gst_app_sink_try_pull_sample(GST_APP_SINK(appsink), long_timeout_ns)) != NULL)
    {
        if (*outCaps == NULL)
        {
            GstCaps *c = gst_sample_get_caps(sample);
            if (c != NULL)
                *outCaps = gst_caps_copy(c);
        }

        GstBuffer *buf = gst_sample_get_buffer(sample);
        if (buf != NULL)
        {
            g_queue_push_tail(queue, gst_buffer_ref(buf));
            count++;
        }
        gst_sample_unref(sample);
    }
    
    /* Check if stream ended properly with EOS */
    if (gst_app_sink_is_eos(GST_APP_SINK(appsink)))
    {
        if (count > 0)
            printf("\n[tdksrc] tdkDrainAppsink: Stream reached EOS, extracted %u buffers\n", count);
    }
    
    return count;
}

static gboolean tdkDemuxContainer(const char *fileUrl, TdkFeedData *feed)
{
    const char *filePrefix = "file:";
    const char *filePath = fileUrl;
    if (strncmp(fileUrl, filePrefix, strlen(filePrefix)) == 0)
        filePath = fileUrl + strlen(filePrefix);

    /* For MP3 files, use raw ES extraction with id3demux + mpegaudioparse */
    if (tdkIsMp3(fileUrl))
    {
        return tdkDemuxMp3WithES(fileUrl, feed);
    }

    /* For other audio-only files (WAV, FLAC, AAC, OGG), use decodebin */
    if (tdkIsAudioOnly(fileUrl))
    {
        printf("\n[tdksrc] demux: audio-only file detected, using direct audio pipeline\n");
        
        GstElement *pipeline = gst_pipeline_new("tdk-audio-demux");
        GstElement *filesrc  = gst_element_factory_make("filesrc", NULL);
        GstElement *decodebin = gst_element_factory_make("decodebin", NULL);
        GstElement *asink    = gst_element_factory_make("appsink", NULL);

        if (!pipeline || !filesrc || !decodebin || !asink)
        {
            printf("\n[tdksrc] demux: failed to create audio extraction elements\n");
            if (pipeline) gst_object_unref(pipeline);
            if (decodebin) gst_object_unref(decodebin);
            if (asink) gst_object_unref(asink);
            return FALSE;
        }

        g_object_set(filesrc, "location", filePath, NULL);
        g_object_set(asink, "sync", FALSE, "async", FALSE, "max-buffers", 0, "emit-signals", FALSE, NULL);

        gst_bin_add_many(GST_BIN(pipeline), filesrc, decodebin, asink, NULL);

        if (!gst_element_link(filesrc, decodebin))
        {
            printf("\n[tdksrc] demux: failed to link filesrc -> decodebin\n");
            gst_object_unref(pipeline);
            return FALSE;
        }

        /* Use heap-allocated struct to avoid use-after-free when signal fires asynchronously */
        TdkDemuxSinks *sinks = (TdkDemuxSinks*)g_malloc(sizeof(TdkDemuxSinks));
        sinks->videoSink = NULL;
        sinks->audioSink = asink;
        gulong signal_id = g_signal_connect(decodebin, "pad-added", G_CALLBACK(tdkDemuxPadAdded), sinks);

        if (gst_element_set_state(pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
        {
            printf("\n[tdksrc] demux: failed to start audio extraction pipeline\n");
            g_signal_handler_disconnect(decodebin, signal_id);
            g_free(sinks);
            gst_object_unref(pipeline);
            return FALSE;
        }

        guint ac = tdkDrainAppsink(asink, feed->audio.app.auQueue, &feed->audioCaps);

        gst_element_set_state(pipeline, GST_STATE_NULL);
        g_signal_handler_disconnect(decodebin, signal_id);
        g_free(sinks);
        gst_object_unref(pipeline);

        printf("\n[tdksrc] demux: extracted 0 video AUs, %u audio AUs from %s\n",
                ac, filePath);

        feed->video.app.demuxed = FALSE;
        feed->audio.app.demuxed = (ac > 0);
        return (ac > 0);
    }

    /* For container files, detect format and create appropriate demuxer */
    GstElement *pipeline = gst_pipeline_new("tdk-demux");
    GstElement *filesrc  = gst_element_factory_make("filesrc", NULL);
    
    /* Choose demuxer based on file extension */
    GstElement *demux = NULL;
    if (strstr(filePath, ".mkv") || strstr(filePath, ".mka") || strstr(filePath, ".webm") || strstr(filePath, ".webmv") || strstr(filePath, ".webma") ||
        strstr(filePath, ".MKV") || strstr(filePath, ".MKA") || strstr(filePath, ".WEBM") || strstr(filePath, ".WEBMV") || strstr(filePath, ".WEBMA"))
        demux = gst_element_factory_make("matroskademux", NULL);  /* Matroska/WebM (WebM is Matroska-based) */
    else
        demux = gst_element_factory_make("qtdemux", NULL);  /* MP4/MOV default */
    
    GstElement *vsink    = gst_element_factory_make("appsink", NULL);
    GstElement *asink    = gst_element_factory_make("appsink", NULL);

    if (!pipeline || !filesrc || !demux || !vsink || !asink)
    {
        printf("\n[tdksrc] demux: failed to create extraction elements\n");
        if (pipeline) gst_object_unref(pipeline);
        return FALSE;
    }

    g_object_set(filesrc, "location", filePath, NULL);
    g_object_set(vsink, "sync", FALSE, "async", FALSE, "max-buffers", 0, "emit-signals", FALSE, NULL);
    g_object_set(asink, "sync", FALSE, "async", FALSE, "max-buffers", 0, "emit-signals", FALSE, NULL);

    /* Use heap-allocated struct to avoid use-after-free when signal fires asynchronously */
    TdkDemuxSinks *sinks = (TdkDemuxSinks*)g_malloc(sizeof(TdkDemuxSinks));
    sinks->videoSink = vsink;
    sinks->audioSink = asink;
    
    gst_bin_add_many(GST_BIN(pipeline), filesrc, demux, vsink, asink, NULL);
    if (!gst_element_link(filesrc, demux))
    {
        printf("\n[tdksrc] demux: failed to link filesrc -> demuxer (qtdemux/matroskademux)\n");
        g_free(sinks);
        gst_object_unref(pipeline);
        return FALSE;
    }
    gulong signal_id = g_signal_connect(demux, "pad-added", G_CALLBACK(tdkDemuxPadAdded), sinks);

    if (gst_element_set_state(pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
    {
        printf("\n[tdksrc] demux: failed to start extraction pipeline\n");
        g_signal_handler_disconnect(demux, signal_id);
        g_free(sinks);
        gst_object_unref(pipeline);
        return FALSE;
    }

    guint vc = tdkDrainAppsink(vsink, feed->video.app.auQueue, &feed->videoCaps);
    guint ac = tdkDrainAppsink(asink, feed->audio.app.auQueue, &feed->audioCaps);

    generate_dot_graph(pipeline, "demux", current_test_name.c_str());

    gst_element_set_state(pipeline, GST_STATE_NULL);
    g_signal_handler_disconnect(demux, signal_id);
    g_free(sinks);
    gst_object_unref(pipeline);

    printf("\n[tdksrc] demux: extracted %u video AUs, %u audio AUs from %s\n",
            vc, ac, filePath);

    feed->video.app.demuxed = (vc > 0);
    feed->audio.app.demuxed = (ac > 0);
    return (vc > 0 || ac > 0);
}

static void tdkSourceSetup(GstElement *playbin, GstElement *source, gpointer userData)
{
    TdkFeedData *feed = (TdkFeedData *) userData;

    if (!GST_IS_TDK_SRC(source))
    {
        printf("\n[tdksrc] source-setup: unexpected source element %s\n", GST_ELEMENT_NAME(source));
        return;
    }

    if (feed->video.app.appsrc != NULL && feed->audio.app.appsrc != NULL)
    {
        printf("\n[tdksrc] source-setup: ignoring subsequent callback\n");
        return;
    }

    printf("\nGot source setup callback\n");
    /* Only get appsrc elements for streams we're actually using */
    if (!only_audio)
        feed->video.app.appsrc = gst_tdk_src_get_video_appsrc(GST_TDK_SRC(source));
    if (!only_video)
        feed->audio.app.appsrc = gst_tdk_src_get_audio_appsrc(GST_TDK_SRC(source));

    if (feed->video.app.demuxed && !only_audio && feed->video.app.appsrc != NULL)
    {
        if (feed->videoCaps != NULL)
        {
            /* Set format to TIME for timestamp-based seeking */
            gst_base_src_set_format(GST_BASE_SRC(feed->video.app.appsrc), GST_FORMAT_TIME);
            
            g_object_set(feed->video.app.appsrc, 
                         "caps", feed->videoCaps,
                         "stream-type", GST_APP_STREAM_TYPE_SEEKABLE,
                         "format", GST_FORMAT_TIME,
                         "block", FALSE,
                         "is-live", FALSE,
                         "do-timestamp", FALSE,
                         NULL);
            
            /* Redundant set for safety (matches MPB pattern) */
            gst_app_src_set_stream_type(GST_APP_SRC(feed->video.app.appsrc), GST_APP_STREAM_TYPE_SEEKABLE);
            
            /* Set size for duration query support */
            guint64 total_size = 0;
            GList *iter = feed->video.app.auQueue->head;
            while (iter != NULL) {
                if (iter->data != NULL)
                {
                    GstBuffer *buf = (GstBuffer *)iter->data;
                    if (GST_IS_BUFFER(buf))
                        total_size += gst_buffer_get_size(buf);
                }
                iter = iter->next;
            }
            g_object_set(feed->video.app.appsrc, "size", (gint64)total_size, NULL);
            
            /* Register seek-data callback for video appsrc */
            g_signal_connect(feed->video.app.appsrc, "seek-data",
                             G_CALLBACK(tdkSeekData), &feed->video.app);
            
            /* Register need-data callback for dynamic buffer feeding (enables seeking) */
            g_signal_connect(feed->video.app.appsrc, "need-data",
                             G_CALLBACK(tdkNeedData), &feed->video.app);
            
            gst_app_src_set_caps(GST_APP_SRC(feed->video.app.appsrc), feed->videoCaps);
            feed->video.app.caps = gst_caps_copy(feed->videoCaps);
            feed->video.app.current_buffer_iter = feed->video.app.auQueue->head;
            feed->video.app.current_byte_offset = 0;
            printf("\n[tdksrc] source-setup: video demuxed caps set, stream-type=SEEKABLE, format=TIME, seek-data callback registered\n");
        }
    }

    if (feed->audio.app.demuxed && !only_video && feed->audio.app.appsrc != NULL)
    {
        if (feed->audioCaps != NULL)
        {
            /* Set format to TIME for timestamp-based seeking */
            gst_base_src_set_format(GST_BASE_SRC(feed->audio.app.appsrc), GST_FORMAT_TIME);
            
            g_object_set(feed->audio.app.appsrc, 
                         "caps", feed->audioCaps,
                         "stream-type", GST_APP_STREAM_TYPE_SEEKABLE,
                         "format", GST_FORMAT_TIME,
                         "block", FALSE,
                         "is-live", FALSE,
                         "do-timestamp", FALSE,
                         NULL);
            
            /* Redundant set for safety (matches MPB pattern) */
            gst_app_src_set_stream_type(GST_APP_SRC(feed->audio.app.appsrc), GST_APP_STREAM_TYPE_SEEKABLE);
            
            /* Set size for duration query support */
            guint64 total_size = 0;
            GList *iter = feed->audio.app.auQueue->head;
            while (iter != NULL) {
                if (iter->data != NULL)
                {
                    GstBuffer *buf = (GstBuffer *)iter->data;
                    if (GST_IS_BUFFER(buf))
                        total_size += gst_buffer_get_size(buf);
                }
                iter = iter->next;
            }
            g_object_set(feed->audio.app.appsrc, "size", (gint64)total_size, NULL);
            
            /* Register seek-data callback for audio appsrc */
            g_signal_connect(feed->audio.app.appsrc, "seek-data",
                             G_CALLBACK(tdkSeekData), &feed->audio.app);
            
            /* Register need-data callback for dynamic buffer feeding (enables seeking) */
            g_signal_connect(feed->audio.app.appsrc, "need-data",
                             G_CALLBACK(tdkNeedData), &feed->audio.app);
            
            gst_app_src_set_caps(GST_APP_SRC(feed->audio.app.appsrc), feed->audioCaps);
            feed->audio.app.caps = gst_caps_copy(feed->audioCaps);
            feed->audio.app.current_buffer_iter = feed->audio.app.auQueue->head;
            feed->audio.app.current_byte_offset = 0;
            printf("\n[tdksrc] source-setup: audio demuxed caps set, stream-type=SEEKABLE, format=TIME, seek-data callback registered\n");
        }
        else
        {
            printf("\n[tdksrc] source-setup: audio demuxed but no caps available\n");
        }
    }

    printf("\n[tdksrc] source-setup: obtained tdksrc (video appsrc=%p, audio appsrc=%p)\n",
            (void *) feed->video.app.appsrc, (void *) feed->audio.app.appsrc);
}

static gboolean setupInlineMultiAppsrc(MessageHandlerData *data, bool play_after_setup)
{
    GstState cur_state;
    GstStateChangeReturn state_change;
    GstFlowReturn ret;

    /* Static feed for multiappsrc - persists between calls for channel changes */
    static TdkFeedData feed = {0};

    /* Clear old feed from previous channel if needed */
    if (feed.video.app.auQueue != NULL)
    {
        g_queue_free_full(feed.video.app.auQueue, (GDestroyNotify)gst_buffer_unref);
        feed.video.app.auQueue = NULL;
    }
    if (feed.audio.app.auQueue != NULL)
    {
        g_queue_free_full(feed.audio.app.auQueue, (GDestroyNotify)gst_buffer_unref);
        feed.audio.app.auQueue = NULL;
    }
    if (feed.videoCaps != NULL)
    {
        gst_caps_unref(feed.videoCaps);
        feed.videoCaps = NULL;
    }
    if (feed.audioCaps != NULL)
    {
        gst_caps_unref(feed.audioCaps);
        feed.audioCaps = NULL;
    }

    memset(&feed, 0, sizeof(feed));
    feed.video.playbin = NULL;
    feed.audio.playbin = NULL;

    if (!gst_element_factory_find("tdksrc"))
    {
        if (!gst_element_register(NULL, "tdksrc", GST_RANK_PRIMARY + 100, GST_TDK_TYPE_SRC))
            return FALSE;
        printf("\n[tdksrc] Registered custom element 'tdksrc'\n");
    }

    /* Support both container files (MP4/MOV/M4V/M4A) and audio-only files (MP3/WAV/FLAC) */
    if (!tdkIsContainer(m_play_url) && !tdkIsAudioOnly(m_play_url))
        return FALSE;

    feed.video.app.auQueue = g_queue_new();
    feed.audio.app.auQueue = g_queue_new();

    if (!tdkDemuxContainer(m_play_url, &feed))
        goto cleanup_fail;

    /* Automatically detect audio-only and video-only streams based on demux results */
    if (!feed.video.app.demuxed && feed.audio.app.demuxed)
    {
        only_audio = true;
        printf("\n[tdksrc] Detected audio-only stream (no video extracted)\n");
    }
    else if (feed.video.app.demuxed && !feed.audio.app.demuxed)
    {
        only_video = true;
        printf("\n[tdksrc] Detected video-only stream (no audio extracted)\n");
    }

    if (feed.videoCaps != NULL)
    {
        gchar *capsStr = gst_caps_to_string(feed.videoCaps);
        printf("\n[tdksrc] Demuxed video caps = %s\n", capsStr);
        g_free(capsStr);
    }
    if (feed.audioCaps != NULL)
    {
        gchar *capsStr = gst_caps_to_string(feed.audioCaps);
        printf("\n[tdksrc] Demuxed audio caps = %s\n", capsStr);
        g_free(capsStr);
    }

    g_object_set(data->playbin, "uri", "tdk://", NULL);
    g_signal_connect(data->playbin, "source-setup", G_CALLBACK(tdkSourceSetup), &feed);

    assert_failure(data->playbin, gst_element_set_state(data->playbin, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE,
            "Failed to set pipeline to playing state", __FUNCTION__, __LINE__, "Setting pipeline to playing state");

    assert_failure(data->playbin,
            (only_audio || feed.video.app.appsrc != NULL) && (only_video || feed.audio.app.appsrc != NULL),
            "Failed to obtain the tdksrc appsrc elements from source-setup", __FUNCTION__, __LINE__,
            "Verify tdksrc appsrc are available");

    assert_failure(data->playbin, gst_element_set_state(data->playbin, GST_STATE_PAUSED) != GST_STATE_CHANGE_FAILURE,
            "Failed to set pipeline to paused state", __FUNCTION__, __LINE__, "Setting pipeline to paused state");

    /* Do NOT call read_data() here - it races with need-data callbacks already connected in tdkSourceSetup.
       The callbacks will handle feeding buffers on-demand from the queues.
       For audio-only, explicitly emit EOS if video stream not present. */
    if (only_video && feed.audio.app.appsrc != NULL)
    {
        printf("\n[tdksrc] Video-only mode: explicitly emitting EOS to audio stream\n");
        g_signal_emit_by_name(feed.audio.app.appsrc, "end-of-stream", &ret);
    }
    if (only_audio && feed.video.app.appsrc != NULL)
    {
        printf("\n[tdksrc] Audio-only mode: explicitly emitting EOS to video stream\n");
        g_signal_emit_by_name(feed.video.app.appsrc, "end-of-stream", &ret);
    }

    if (!play_after_setup)
        goto cleanup_success;

    assert_failure(data->playbin, gst_element_set_state(data->playbin, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE,
            "Failed to set pipeline to playing state", __FUNCTION__, __LINE__, "Setting pipeline to playing state (after feed)");

    do {
        state_change = gst_element_get_state(data->playbin, &cur_state, NULL, 10000000);
    } while (state_change == GST_STATE_CHANGE_ASYNC);
    printf("\n[tdksrc] Pipeline set to : %s state\n", gst_element_state_get_name(cur_state));
    WaitForOperation;

    assert_failure(data->playbin, cur_state == GST_STATE_PLAYING, "Pipeline is not set to playing state",
            __FUNCTION__, __LINE__, "Verifying if pipeline is successfully set to PLAYING state");

cleanup_success:
    /* Disconnect source-setup handler to prevent re-registration on state changes */
    g_signal_handlers_disconnect_by_func(data->playbin, (gpointer)tdkSourceSetup, &feed);
    
    /* For non-trickplay (play_after_setup=false): disconnect all callbacks and free queues.
       For trickplay (play_after_setup=true): keep queues intact for next channel/operation. */
    if (!play_after_setup)
    {
        /* Disconnect need-data and seek-data callbacks to prevent dangling pointer access */
        if (feed.video.app.appsrc != NULL)
        {
            g_signal_handlers_disconnect_by_func(feed.video.app.appsrc, (gpointer)tdkNeedData, &feed.video.app);
            g_signal_handlers_disconnect_by_func(feed.video.app.appsrc, (gpointer)tdkSeekData, &feed.video.app);
        }
        if (feed.audio.app.appsrc != NULL)
        {
            g_signal_handlers_disconnect_by_func(feed.audio.app.appsrc, (gpointer)tdkNeedData, &feed.audio.app);
            g_signal_handlers_disconnect_by_func(feed.audio.app.appsrc, (gpointer)tdkSeekData, &feed.audio.app);
        }
        
        /* Free queues and caps only when not using them for trickplay */
        if (feed.video.app.auQueue != NULL)
        {
            g_queue_free_full(feed.video.app.auQueue, (GDestroyNotify) gst_buffer_unref);
            feed.video.app.auQueue = NULL;
        }
        if (feed.audio.app.auQueue != NULL)
        {
            g_queue_free_full(feed.audio.app.auQueue, (GDestroyNotify) gst_buffer_unref);
            feed.audio.app.auQueue = NULL;
        }
    }
    /* For trickplay mode: keep queues alive for next operation */
    
    if (feed.videoCaps != NULL)
        gst_caps_unref(feed.videoCaps);
    if (feed.audioCaps != NULL)
        gst_caps_unref(feed.audioCaps);
    g_free(feed.video.app.data);
    g_free(feed.audio.app.data);
    return TRUE;

cleanup_fail:
    /* Cleanup failed before pipeline was fully created */
    g_signal_handlers_disconnect_by_func(data->playbin, (gpointer)tdkSourceSetup, &feed);
    
    /* Disconnect all callbacks to prevent dangling pointer access */
    if (feed.video.app.appsrc != NULL)
    {
        g_signal_handlers_disconnect_by_func(feed.video.app.appsrc, (gpointer)tdkNeedData, &feed.video.app);
        g_signal_handlers_disconnect_by_func(feed.video.app.appsrc, (gpointer)tdkSeekData, &feed.video.app);
    }
    if (feed.audio.app.appsrc != NULL)
    {
        g_signal_handlers_disconnect_by_func(feed.audio.app.appsrc, (gpointer)tdkNeedData, &feed.audio.app);
        g_signal_handlers_disconnect_by_func(feed.audio.app.appsrc, (gpointer)tdkSeekData, &feed.audio.app);
    }
    
    if (feed.videoCaps != NULL)
        gst_caps_unref(feed.videoCaps);
    if (feed.audioCaps != NULL)
        gst_caps_unref(feed.audioCaps);
    
    /* Setup failed before pipeline creation, buffers never reached pipeline.
       Safe to cleanup and release all refs (demux ref + protection ref). */
    if (!only_audio && feed.video.app.auQueue != NULL)
    {
        g_queue_free_full(feed.video.app.auQueue, (GDestroyNotify) gst_buffer_unref);
        feed.video.app.auQueue = NULL;
    }
    if (!only_video && feed.audio.app.auQueue != NULL)
    {
        g_queue_free_full(feed.audio.app.auQueue, (GDestroyNotify) gst_buffer_unref);
        feed.audio.app.auQueue = NULL;
    }
        g_queue_free_full(feed.audio.app.auQueue, (GDestroyNotify) gst_buffer_unref);
    g_free(feed.video.app.data);
    g_free(feed.audio.app.data);
    return FALSE;
}

/*************************************************** tdksrc reading end *******************************************/

/********************************************************************************************************************
Purpose:               Setup pipeline with playbin and westerossink
*********************************************************************************************************************/
static void SetupPipeline (MessageHandlerData *data, bool play_after_setup = true)
{
    GstElement *playsink;
    GstStateChangeReturn state_change;
    std::string readFile;
    const char* filePrefix = "file:";
    bool multiappsrcPlayback = false;

    if (log_enabled)
    {
        fprintf (file,"\n########################\n");
        fprintf (file,"# Setup Pipeline Start\n");
        fprintf (file,"########################\n");
    }
    /*
     * Create the playbin element
     */
    data->playbin = gst_element_factory_make(PLAYBIN_ELEMENT, NULL);
    assert_failure(data->playbin, data->playbin != NULL, "Failed to create 'playbin' element",__FUNCTION__,__LINE__,"Create Playbin Instance");
    DOT_GENERATE_SETUP(data->playbin);
    /*
     * Set the url received from argument as the 'uri' for playbin
     */
    assert_failure (data->playbin, m_play_url != NULL, "Playback url should not be NULL");
    if (log_enabled)
        fprintf(file, "URL is set to %s\n\n", m_play_url);
   
    if (strncmp(m_play_url, filePrefix, strlen(filePrefix)) == 0)
    {
	const char* file_path = m_play_url + strlen(filePrefix);
	fileExists(file_path);
    }

    if (force_appsrc)
	use_appsrc = true;

    if (use_multiappsrc && force_appsrc)
    {
        printf("\nMulti-appsrc path is skipped for manual underflow/appsrc tests\n");
        use_multiappsrc = false;
    }

    // Appsrc implementation skipped for ABR streams
    if (use_appsrc || use_multiappsrc)
    {
        const char *suffix1 = ".m3u8";
        const char *suffix2 = ".mpd";
	if (suffixCompare(m_play_url, suffix1) || suffixCompare(m_play_url, suffix2)) 
	{
            printf("\nAppsrc implementation not supported for ABR streams");
	    printf("\nSwitching to conventional playbin technique\n");
	    use_appsrc = false;
	    use_multiappsrc = false;
	}
    }

    if (use_multiappsrc)
    {
        multiappsrcPlayback = true;
    }
    else if (use_appsrc)
    {
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
	if (getenv ("TDK_NATIVE_AUDIO") != NULL)
        {
            flags |= GST_PLAY_FLAG_NATIVE_AUDIO;
            printf("\nEnabled NATIVE_AUDIO flag for playbin\n");
        }
    }
    else if (only_video)
    {
	flags = GST_PLAY_FLAG_VIDEO;
	if (getenv ("TDK_NATIVE_VIDEO") != NULL)
        {
            flags |= GST_PLAY_FLAG_NATIVE_VIDEO;
            printf("\nEnabled NATIVE_VIDEO flag for playbin\n");
        }
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
     * Set westeros-sink (skip for audio-only playback)
     */
    if (!only_audio)
    {
        data->westerosSink.sink = gst_element_factory_make(WESTEROS_SINK, NULL);
        assert_failure (data->playbin, data->westerosSink.sink != NULL, "Failed to create 'westerossink' element",__FUNCTION__,__LINE__,"Create Westeros Instance");
    }
    else
    {
        data->westerosSink.sink = NULL;
    }

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
     * Link the westeros-sink to playbin (skip for audio-only playback)
     */
    if (!only_audio)
    {
        g_object_set (data->playbin, "video-sink", data->westerosSink.sink, NULL);
    }
    g_object_set (data->playbin, "async-handling", true, NULL);

    g_signal_connect(data->playbin, "element-setup", G_CALLBACK(elementSetupCallback), &data);
    /*
     * Set the first frame received callback (only for video playback)
     */
    if (data->westerosSink.sink != NULL)
    {
        g_signal_connect(data->westerosSink.sink, "first-video-frame-callback", G_CALLBACK(firstFrameCallback), &firstFrameReceived);
    }
    /*
     * Set the firstFrameReceived variable as false before starting play
     */
    firstFrameReceived= false;

    if (data->westerosSink.sink != NULL)
    {
        if (enable_last_sample)
        {
            g_object_set(data->westerosSink.sink, "enable-last-sample", TRUE, NULL);
            printf ("\n Last Sample Enabled\n");
        }

        g_signal_connect(data->westerosSink.sink, "buffer-underflow-callback", G_CALLBACK(bufferUnderflowCallback), &videoUnderflowReceived);
    }

    data->pipelineInitiation = true;
    data->westerosSink.previous_rendered_frames = 0;
    data->audioSink.previous_rendered_frames = 0;
    data->current_bandwidth = 0;

    if (multiappsrcPlayback)
    {
        if (!setupInlineMultiAppsrc(data, play_after_setup))
        {
            printf("\n[tdksrc] multi-appsrc setup failed, falling back to conventional playbin uri path\n");
            g_object_set(data->playbin, "uri", m_play_url, NULL);
            multiappsrcPlayback = false;
            use_multiappsrc = false;
            use_appsrc = false;
        }
        else if (!play_after_setup)
        {
            return;
        }
    }

    /*
     * Set playbin to PLAYING
     */
    if (!play_after_setup)
	return;

    if (multiappsrcPlayback)
	goto post_start_validation;

    if (log_enabled)
    {
        fprintf (file,"\n###################################\n");
        fprintf (file,"# Setting Pipeline to PLAYING State\n");
        fprintf (file,"###################################\n");
    }
    GST_FIXME( "Setting to Playing State\n");
    assert_failure (data->playbin, gst_element_set_state (data->playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE,"Failed to set pipeline to playing state",__FUNCTION__,__LINE__,"Setting pipeline to playing state");
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
	assert_failure (data->playbin, data->cur_state == GST_STATE_PLAYING, "Pipeline is not set to playing state", __FUNCTION__,__LINE__,"Verifying if pipeline is successfully set to PLAYING state");
    }	

    post_start_validation:
    /*
     * Check if the first frame received flag is set
     */
    assert_failure (data->playbin, (only_audio) || (firstFrameReceived == true), "Failed to receive first video frame signal", __FUNCTION__,__LINE__,"Verify if first frame signal is received");
    audio_underflow_received =  false;
    videoUnderflowReceived =  false;
    
    if (!only_audio)
    {
         g_object_get (data->westerosSink.sink, "video-height", &height, NULL);
         g_object_get (data->westerosSink.sink, "video-width", &width, NULL);

         printf("\nVideo height = %d\nVideo width = %d", height, width);
	 if (log_enabled)
	     fprintf(file, "\nVideo height = %d\nVideo width = %d", height, width);
    }
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
    assert_failure(playbin, playbin != NULL, "Failed to create 'playbin' element",__FUNCTION__,__LINE__,"Create Playbin Instance");
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
    assert_failure (playbin, westerosSink != NULL, "Failed to create 'westerossink' element",__FUNCTION__,__LINE__,"Create Westeros Instance");

    /*
     * Link the westeros-sink to playbin
     */
    g_object_set (playbin, "video-sink", westerosSink, NULL);

    terminatePipeline(playbin);
}
GST_END_TEST;

/********************************************************************************************************************
 * Purpose      : Test to calculate the playback latency from westeros sink to display renderer
 * Parameters   : Playback url
 ********************************************************************************************************************/
GST_START_TEST (test_playback_latency)
{
    MessageHandlerData data;
    GstState cur_state;
    GstStateChangeReturn state_change;
    gint64 currentPosition;

    /* Setup pipeline without auto-playing - we'll set PLAYING state manually for latency measurement */
    SetupPipeline(&data, false);

    assert_failure(data.playbin, gst_element_set_state(data.playbin, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE,
            "Failed to set pipeline to playing state", __FUNCTION__, __LINE__, "Set Pipeline to PLAYING state");

    do {
        state_change = gst_element_get_state(data.playbin, &cur_state, NULL, 10000000);
    } while (state_change == GST_STATE_CHANGE_ASYNC);

    assert_failure(data.playbin, cur_state == GST_STATE_PLAYING, "Failed to set to playing state",
            __FUNCTION__, __LINE__, "Verifying if pipeline is successfully set to PLAYING state");
    printf("\n********Current state is: %s \n", gst_element_state_get_name(cur_state));

    clock_t start_time = clock();
    const double max_loop_time = 3.0;

    auto start_latency = std::chrono::high_resolution_clock::now();
    while ((clock() - start_time) / CLOCKS_PER_SEC < max_loop_time)
    {
        bool ret_value = gst_element_query_position(data.playbin, GST_FORMAT_TIME, &currentPosition);

        if (!ret_value)
            assert_failure(data.playbin, ret_value, "Failed to query the current playback position",
                    __FUNCTION__, __LINE__, "Get Pipeline Position");

        if ((int)(currentPosition/GST_SECOND) >= 1)
        {
            assert_failure(data.playbin, ret_value, "Failed to query the current playback position",
                    __FUNCTION__, __LINE__, "Get Pipeline Position");
            stop_latency = std::chrono::high_resolution_clock::now();
            //Since playback already happended for 1 second, reduce from stop_latency
            stop_latency -= std::chrono::seconds(1);
            break;
        }
    }

    assert_failure(data.playbin, firstFrameReceived == true, "Failed to receive first video frame signal",
            __FUNCTION__, __LINE__, "Verify if first frame signal is received");
    auto New_latency = std::chrono::duration_cast<std::chrono::milliseconds>(stop_latency - start_latency);
    printf("\nTime measured: %.3lld milliseconds.\n", New_latency.count());
    if (log_enabled)
        fprintf(file, "\nTime measured: %.3lld milliseconds.\n", New_latency.count());

    int latency_int = New_latency.count();
    /*
     * Writing to file
     */
    FILE *filePointer;
    char latency_file[BUFFER_SIZE_SHORT] = {'\0'};
    strcat(latency_file, TDK_PATH);
    strcat(latency_file, "/latency_log");
    filePointer = fopen(latency_file, "w");
    if (filePointer != NULL)
    {
        fprintf(filePointer, "Latency = %d milliseconds\n", latency_int);
    }
    else
    {
        printf("\nLatency writing operation failed\n");
    }
    fclose(filePointer);

    Sleep(2);

    terminatePipeline(data.playbin);
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

    /* Disable multiappsrc for channel change tests - incompatible with feed switching */
    if (ChannelChangeTest)
    {
        printf("\nDisabling multiappsrc for channel change test (incompatible with feed switching)\n");
        use_multiappsrc = false;
    }

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
    if (log_enabled)
	fprintf(file,"\nVideo height = %d\nVideo width = %d", height, width);

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
	     assert_failure (data.playbin,to_string(height) == resolution,resolution_value_string, __FUNCTION__,__LINE__, "Verify if pipeline is playin expected resolution");
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
	 assert_failure (data.playbin,ResolutionCount == TOTAL_RESOLUTIONS_COUNT,"\nNot able to play all resolutions\n", __FUNCTION__,__LINE__,
			  "Verify if pipeline played all resolutions");
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
        assert_failure (data.playbin,is_av_playing == true, "Video is not playing in TV",  __FUNCTION__,__LINE__, "Verify video playback using proc entry");
    }

    printf("DETAILS: SUCCESS, Video playing successfully \n");

    if (data.playbin)
    {
       assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_NULL) !=  GST_STATE_CHANGE_FAILURE,"Failed to set to NULL state", __FUNCTION__,__LINE__,
		       "Set Pipeline to NULL state");
    }
    if (ChannelChangeTest)
    {
       /* Declare feed at block scope for multiappsrc channel change */
       TdkFeedData feed = {0};

       /*
        * Set the url received from argument as the 'channel_url' for playbin
        */
       assert_failure (data.playbin,channel_url != NULL, "Second Channel url should not be NULL");
       
       printf("\nLoading Second Channel\n");
       if (log_enabled)
	   fprintf(file,"\nLoading second URL : %s \n\n", channel_url);

       if (use_multiappsrc)
       {
           /* Multiappsrc channel change: Try container demux, fallback to standard if needed */
           printf("\nMultiappsrc Channel Change: Attempting new file load\n");
           
           /* Check if second channel is also a container */
           if (!tdkIsContainer(channel_url))
           {
               printf("WARNING: Second channel is not a container, falling back to standard playbin URI switching\n");
               g_object_set (data.playbin, "uri", channel_url, NULL);
               data.streamStart = FALSE;
               data.terminate = FALSE;
               use_multiappsrc = FALSE;  /* Disable multiappsrc for this channel change */
           }
           else
           {
               /* Stop pushing from first channel */
               data.terminate = true;
               while(read_data(&data));
               printf("Stopped pushing first channel data\n");
               
               /* Initialize queues for new feed */
               feed.video.app.auQueue = g_queue_new();
               feed.audio.app.auQueue = g_queue_new();
               
               /* Demux new channel file */
               if (tdkDemuxContainer(channel_url, &feed))
               {
                   printf("Successfully demuxed new channel file\n");
                   data.streamStart = FALSE;
                   data.terminate = FALSE;
               }
               else
               {
                   printf("ERROR: Failed to demux second channel file, falling back to standard playbin\n");
                   g_object_set (data.playbin, "uri", channel_url, NULL);
                   data.streamStart = FALSE;
                   data.terminate = FALSE;
                   use_multiappsrc = FALSE;  /* Disable multiappsrc for this channel change */
               }
           }
       }
       else
       {
           /* Standard playbin channel change via URI */
           g_object_set (data.playbin, "uri", channel_url, NULL);
           data.streamStart= FALSE;
           data.terminate = FALSE;
       }


       if (use_multiappsrc)
       {
           /* For multiappsrc, pipeline already PLAYING - just resume pushing new feed data */
           printf("\nMultiappsrc: Resuming data push for second channel\n");
           if (!only_audio)
               while(read_data(&feed.video));
           if (!only_video)
               while(read_data(&feed.audio));
           printf("Finished pushing second channel data\n");
           data.streamStart = TRUE;  /* Mark stream as started */
       }
       else
       {
           /*
            * Set playbin to PLAYING (for standard playbin URI change)
            */
           GST_LOG( "Setting Second Channel to Playing State\n");
           assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE,"Failed to set to playing state",
                           __FUNCTION__,__LINE__,"Set Pipeline to PLAYING state");
           GST_LOG( "Second Channel Set to Playing State\n");
       }
       
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
       assert_failure (data.playbin,FALSE == data.terminate, "Unexpected error or End of Stream received\n", __FUNCTION__,__LINE__,
		       "Verify no errors were observed in pipeline while loading second url");
       /*
        * Verify that STREAM_START message is received
        */
       assert_failure (data.playbin,TRUE == data.streamStart, "Unable to obtain message indicating start of a new stream\n", __FUNCTION__,__LINE__,
		       "Verify STREAM_START message is received");
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

       assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_NULL) !=  GST_STATE_CHANGE_FAILURE,"Failed to set to NULL state", __FUNCTION__,__LINE__,
                       "Set Pipeline to NULL state");
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
        assert_failure (data.playbin,is_av_playing == true, "Video is not playing in TV", __FUNCTION__,__LINE__, "Verify video playback using proc entry");
    }
    printf ("\nDETAILS: SUCCESS, Playback is successfull\n");

    auto latency_start = std::chrono::high_resolution_clock::now();
    auto latency_stop =  std::chrono::high_resolution_clock::now();
    auto latency_chrono = std::chrono::duration_cast<std::chrono::milliseconds>(latency_stop - latency_start);

    /*
     * Set pipeline to PAUSED
     */
    assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &data.previousposition), "Failed to query the current playback position",
		    __FUNCTION__,__LINE__,"Querying Playback Position" ); 
    assert_failure (data.playbin, gst_element_set_state (data.playbin, GST_STATE_PAUSED) !=  GST_STATE_CHANGE_FAILURE,"Failed to set pipeline to PAUSED state",
		    __FUNCTION__,__LINE__,"Setting pipeline to PAUSED state");
    do{
        state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, 10000000);
    } while (state_change == GST_STATE_CHANGE_ASYNC);
    printf ("\n********Current state is: %s \n", gst_element_state_get_name(data.cur_state));
    assert_failure (data.playbin, data.cur_state == GST_STATE_PAUSED, "Pipeline is not set to PAUSED state",
		    __FUNCTION__,__LINE__,"Verify Pipeline is set to PAUSED State");
    printf ("\nDETAILS: SUCCESS, Current state is: %s \n", gst_element_state_get_name(data.cur_state));

    if (checkNewPlay)
        PlaybackValidation(&data,play_timeout);
    else
        PlaySeconds(data.playbin,play_timeout);

    assert_failure (data.playbin, gst_element_get_state (data.playbin, &data.cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS,"Failed to obatin pipeline state",
		    __FUNCTION__,__LINE__,"Get Pipeline State");
    if ((data.cur_state != GST_STATE_PLAYING))
    {
	assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &data.previousposition), "Failed to query the current playback position",
			__FUNCTION__,__LINE__,"Querying Playback Position");    
        assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "Failed to set to PLAYING state",
			__FUNCTION__,__LINE__,"Setting pipeline to PLAYING state");
        do{
             state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, 10000000);
        } while (state_change == GST_STATE_CHANGE_ASYNC);
	printf ("\n********Current state is: %s \n", gst_element_state_get_name(data.cur_state));
    }

    assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &data.previousposition), "Failed to query the current playback position",
		    __FUNCTION__,__LINE__,"Querying Playback Position");
    if (checkNewPlay)
        PlaybackValidation(&data,play_timeout);
    else
        PlaySeconds(data.playbin,play_timeout);

    terminatePipeline(data.playbin);
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
    assert_failure (data.playbin, gst_element_query_duration(data.playbin, GST_FORMAT_TIME, &duration), "Failed to query playback duration",
		    __FUNCTION__,__LINE__,"Querying Playback Duration");

    assert_failure(data.playbin, (duration != -1), "Obtained duration is not valid",
		    __FUNCTION__,__LINE__,"Verify playback duration");
   
    terminatePipeline(data.playbin);
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

    assert_failure (data.playbin,videoUnderflowReceived == true, "Failed to receive buffer underflow signal",
		    __FUNCTION__,__LINE__,"Verify buffer underflow signal received");

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

    assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_PAUSED) !=  GST_STATE_CHANGE_FAILURE, "Failed to set pipeline to PAUSED state",
		    __FUNCTION__,__LINE__,"Set Pipeline to PAUSED state");
    do{
       //Waiting for state change
       state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, 10000000);
    } while (state_change == GST_STATE_CHANGE_ASYNC);

    assert_failure (data.playbin, gst_element_get_state (data.playbin, &data.cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS, "Failed to get playback state",
		    __FUNCTION__,__LINE__,"Obtain pipeline state");
    assert_failure (data.playbin, data.cur_state == GST_STATE_PAUSED, "Pipeline is not set to PAUSED state", __FUNCTION__,__LINE__, "Verify Pipeline is set to PAUSED state");
    printf("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));
    GST_LOG("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));
    assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &(data.previousposition)), "Failed to query the current playback position",
		    __FUNCTION__,__LINE__,"Query playback position");
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
                                   GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE), "Failed to seek",
		    __FUNCTION__,__LINE__,"Seek to videoStart Position");

    start = std::chrono::steady_clock::now();
    while(!seeked)
    {
       //Check if seek happened
       assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &(data.currentPosition)),"Failed to query the current playback position",
		       __FUNCTION__,__LINE__,"Query playback position");
       //Added GST_SECOND buffer time between currentPosition and seekPosition
       if (abs( data.currentPosition - seekPosition) <= (GST_SECOND))
       {
           seeked = TRUE;
       }
       if (std::chrono::steady_clock::now() - start > std::chrono::seconds(Seek_time_threshold))
           break;
    }

    assert_failure (data.playbin,TRUE == seeked, "Seek Unsuccessfull\n", __FUNCTION__,__LINE__, "Verify is seek is successfull");
    //Convert time to seconds
    data.currentPosition /= GST_SECOND;
    seekPosition /= GST_SECOND;
    printf("\nSEEK SUCCESSFULL :  CurrentPosition %lld seconds, SeekPosition %lld seconds\n", data.currentPosition, seekPosition);
    if (log_enabled)
	fprintf(file, "\nSEEK SUCCESSFULL :  CurrentPosition %lld seconds, SeekPosition %lld seconds\n", data.currentPosition, seekPosition);

    state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, GST_SECOND);
    assert_failure (data.playbin,state_change != GST_STATE_CHANGE_FAILURE, "Failed to get current playbin state", __FUNCTION__,__LINE__, "Obtain pipeline state");

    if ((data.cur_state == GST_STATE_PAUSED)  && (state_change != GST_STATE_CHANGE_ASYNC))
    {

        /*
         * Set playbin to PLAYING
         */
         GST_FIXME( "Setting to Playing State\n");
         assert_failure (data.playbin,gst_element_set_state(data.playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "Failed to set to playing state",
			 __FUNCTION__,__LINE__, "Set Pipeline to PLAYING state");
         GST_FIXME( "Set to Playing State\n");
         do{
              //Waiting for state change
              state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, 10000000);
         } while (state_change == GST_STATE_CHANGE_ASYNC);

         printf("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));
	 assert_failure (data.playbin, gst_element_get_state (data.playbin, &data.cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS, "Failed to get current playbin state",
			 __FUNCTION__,__LINE__, "Obtain Pipeline State");
         assert_failure (data.playbin, data.cur_state == GST_STATE_PLAYING, "Pipeline not set to PLAYING state",
			 __FUNCTION__,__LINE__, "Verify if pipeline is successfully set to PLAYING state");
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
        assert_failure (data.playbin,is_av_playing == true, "Video is not playing in TV", 
			__FUNCTION__,__LINE__, "Verify video playback using proc entry");
    }

    terminatePipeline(data.playbin);
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
        assert_failure (data.playbin,is_av_playing == true, "Video is not playing in TV",
			__FUNCTION__,__LINE__, "Verify video playback using proc entry");
    }
    printf ("DETAILS: SUCCESS, Video playing successfully \n");

    /*
     * Polling the bus for messages
     */
    bus = gst_element_get_bus (data.playbin);
    assert_failure (data.playbin,bus != NULL,"Failed to get bus", __FUNCTION__,__LINE__, "Get Playbin bus");

    if (log_enabled)
	 fprintf(file, "\nWaiting for pipeline to play video and emit EOS message\n");
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
    assert_failure (data.playbin,received_EOS == true, "Failed to recieve EOS message", __FUNCTION__,__LINE__, "Verify EOS message received");
    printf ("EOS Received\n");
    gst_message_unref (message);

    terminatePipeline(data.playbin);
}
GST_END_TEST;


GST_START_TEST (test_frameDrop)
{
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
    MessageHandlerData data;

    /* Use SetupPipeline to support both standard and multiappsrc paths */
    SetupPipeline(&data);

    assert_failure(data.playbin, firstFrameReceived == true, "Failed to receive first video frame signal",
		    __FUNCTION__,__LINE__,"Verify if first frame signal is received");

    assert_failure(data.playbin, gst_element_query_position(data.playbin, GST_FORMAT_TIME, &currentPosition), 
                   "Failed to query the current playback position", __FUNCTION__,__LINE__,"Query playback position");
    
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

    bus = gst_element_get_bus(data.playbin);
    g_object_get(data.westerosSink.sink, "stats", &structure, NULL);
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
        g_object_get(data.westerosSink.sink, "stats", &structure, NULL);

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
        assert_failure(data.playbin, frame_buffer != 0, "frames are not rendered properly", __FUNCTION__,__LINE__," Verify frames are rendered properly");

	runLoop = gst_element_query_position(data.playbin, GST_FORMAT_TIME, &currentPosition);
	if (!runLoop)
	    gst_object_unref(bus);
        assert_failure(data.playbin, runLoop, "Failed to query the current playback position", __FUNCTION__,__LINE__,"Obtain pipeline position");
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
            assert_failure(data.playbin, rendered_frames > previous_rendered_frames, "Frames not incremented as playback progresses",
			    __FUNCTION__,__LINE__,"Verify if rendered frames are incremented as playback progresses");
	}

	if ((checkTotalFrames) && ((totalFrames - rendered_frames) < MIN_FRAMES_DROP))
            break;

        if (!checkTotalFrames)
	{
	    if (jump_buffer==0)
                gst_object_unref(bus);
            assert_failure(data.playbin, jump_buffer != 0, "Playback is not happening at the expected rate",
			    __FUNCTION__,__LINE__,"Verify if playback is progressing at 1x playback rate");
	}

        message = gst_bus_pop_filtered(bus, (GstMessageType)GST_MESSAGE_EOS);
        if (message != NULL)
        {
            if (GST_MESSAGE_EOS == GST_MESSAGE_TYPE(message))
            {
                printf("\nEOS Received:Exiting\n");
		if (log_enabled)
		    fprintf(file, "\nEOS Received:Exiting\n");
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

    gst_object_unref(bus);
    assert_failure(data.playbin, gst_element_set_state(data.playbin, GST_STATE_PAUSED) != GST_STATE_CHANGE_FAILURE, "Failed to set to PAUSED state",
		    __FUNCTION__,__LINE__,"Set Pipeline to PAUSED state");
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
    if (log_enabled)
	 fprintf(file, "\nExpected rendered frames : %d", expected_rendered_frames);

    if ((int)rendered_frames < expected_rendered_frames)
    {
         dropped_frames = expected_rendered_frames - rendered_frames;
	 printf("\n\ndropped_frames : %d", dropped_frames);
	 if (log_enabled)
	     fprintf(file, "\n\ndropped_frames : %d", dropped_frames);
	 sleep(2);
	 if (dropped_frames)
	     assert_failure(data.playbin, dropped_frames < MIN_FRAMES_DROP,"\nExpected number of frames not rendered\n", __FUNCTION__,__LINE__,"Verify all frames are rendered");
    }
    printf ("\nExecution was Success\n");

    terminatePipeline(data.playbin);
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
    assert_failure (data.playbin,1 < data.n_audio,"Stream has only 1 audio stream. Audio Change requires minimum of two audio streams",
		    __FUNCTION__, __LINE__, "Verify if URL has two audio streams");
    printf("Current Audio is %d\n", data.current_audio);
    if (log_enabled)
	fprintf(file, "\nCurrent Audio is %d\n", data.current_audio);
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
                 assert_failure(data.playbin, gst_element_set_state (data.playbin, GST_STATE_PAUSED) !=  GST_STATE_CHANGE_FAILURE, "Failed to set pipeline to PAUSED state",
				__FUNCTION__, __LINE__, "Set Pipeline to PAUSED state") ;
		 WaitForOperation;
		 seekOperation = false;
		 if (checkNewPlay)
                     PlaybackValidation(&data,2);
                 else
                     PlaySeconds(data.playbin,2);
	     }
             printf("\nSwitching to audio stream %d\n", index);
             printf("\nSetting current-audio to %d\n",index);
	     if (log_enabled)
		 fprintf(file, "\nSwitching to audio stream %d\n", index);
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
                 assert_failure(data.playbin, gst_element_set_state (data.playbin, GST_STATE_PLAYING)  !=  GST_STATE_CHANGE_FAILURE, "Failed to set pipeline to PLAYING state",
                                __FUNCTION__, __LINE__, "Set Pipeline to PLAYING state") ;;
             }
	     assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &(data.currentPosition)), "Failed to query the current playback position",
			     __FUNCTION__, __LINE__, "Query playback position");
             if (Flush_Pipeline)
             {
                 flushPipeline(&data);
             }
             printf("\nSUCCESS : Switched to audio stream %d, Playing for %d seconds\n",index,play_timeout);
	     if (log_enabled)
		 fprintf(file, "\nSUCCESS : Switched to audio stream %d, Playing for %d seconds\n",index,play_timeout);
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
        assert_failure (data.playbin,is_av_playing == true, "Video is not playing in TV", __FUNCTION__,__LINE__, "Verify video playback using proc entry");
    }
    GST_LOG("DETAILS: SUCCESS, Video playing successfully for all audio streams\n");


    terminatePipeline(data.playbin);
    fclose(filePointer);
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
                  assert_failure(data.playbin, data.westerosSink.previous_rendered_frames == data.westerosSink.rendered_frames,"Video is not stopped as expected",
				 __FUNCTION__, __LINE__, "Verify audio frames are incremented as stream progresses");
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

    assert_failure (data.playbin,audio_underflow_received_global == true, "Failed to receive buffer underflow signal",
		    __FUNCTION__, __LINE__, "Verify if audio underflow signal is received");
    	
    printf ("\nDETAILS: SUCCESS, Received buffer underflow signal as expected\n");

    
    if (checkSignalTest || play_without_audio)
         goto EXIT;
	
    bufferUnderflowTest = false;

    assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_PAUSED) !=  GST_STATE_CHANGE_FAILURE, "Failed to PAUSE the pipeline",
		    __FUNCTION__, __LINE__, "Set Pipeline to PAUSED state");
    do{
       //Waiting for state change
       state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, 10000000);
    } while (state_change == GST_STATE_CHANGE_ASYNC);

    assert_failure (data.playbin, gst_element_get_state (data.playbin, &data.cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS);
    assert_failure (data.playbin, data.cur_state == GST_STATE_PAUSED);
    printf("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));
    GST_LOG("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));
    assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &(data.previousposition)), "Failed to query the current playback position",
		    __FUNCTION__, __LINE__, "Query Playback Position");
    if (checkNewPlay)
        PlaybackValidation(&data,5);
    else
        PlaySeconds(data.playbin,5);
    
    assert_failure (data.playbin, gst_element_get_state (data.playbin, &data.cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS, "Failed to get pipeline state",
		    __FUNCTION__, __LINE__, "Get Pipeline state");
    assert_failure (data.playbin, data.cur_state == GST_STATE_PAUSED, "Pipeline is not set to PAUSED state",
		    __FUNCTION__, __LINE__, "Verify if Pipeline is PAUSED");
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
                                   GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE), "Failed to seek",
		    __FUNCTION__, __LINE__, "Seek to audioStart Position");

    start = std::chrono::steady_clock::now();
    while(!seeked)
    {
       //Check if seek happened
       assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &(data.currentPosition)),
                                         "Failed to query the current playback position", __FUNCTION__, __LINE__, "Query playback position");
       //Added GST_SECOND buffer time between currentPosition and seekPosition
       if (abs( data.currentPosition - seekPosition) <= (GST_SECOND))
       {
           seeked = TRUE;
       }
       if (std::chrono::steady_clock::now() - start > std::chrono::seconds(Seek_time_threshold))
           break;
    }

    assert_failure (data.playbin,TRUE == seeked, "Seek Unsuccessfull\n", __FUNCTION__, __LINE__, "Verify if seek is successfull");
    //Convert time to seconds
    data.currentPosition /= GST_SECOND;
    seekPosition /= GST_SECOND;
    printf("\nSEEK SUCCESSFULL :  CurrentPosition %lld seconds, SeekPosition %lld seconds\n", data.currentPosition, seekPosition);

    state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, GST_SECOND);
    assert_failure (data.playbin,state_change != GST_STATE_CHANGE_FAILURE, "Failed to get current playbin state",
		    __FUNCTION__, __LINE__, "Query playbin state");

    if ((data.cur_state == GST_STATE_PAUSED)  && (state_change != GST_STATE_CHANGE_ASYNC))
    {

        /*
         * Set playbin to PLAYING
         */
         GST_FIXME( "Setting to Playing State\n");
         assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "Failed to set to PAUSED state",
			 __FUNCTION__, __LINE__, "Set Pipeline to PAUSED state");
         GST_FIXME( "Set to Playing State\n");
         do{
              //Waiting for state change
              state_change = gst_element_get_state (data.playbin, &data.cur_state, NULL, 10000000);
         } while (state_change == GST_STATE_CHANGE_ASYNC);

         printf("\n********Current state: %s\n",gst_element_state_get_name(data.cur_state));
	 assert_failure (data.playbin, gst_element_get_state (data.playbin, &data.cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS, "Failed to get pipeline state",
			 __FUNCTION__, __LINE__, "Get Pipeline state");
         assert_failure (data.playbin, data.cur_state == GST_STATE_PLAYING, "Failed to set pipeline to PLAYING state",
			 __FUNCTION__, __LINE__, "Verify if Pipeline is set to PLAYING state");
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
        assert_failure (data.playbin,is_av_playing == true, "Video is not playing in TV",__FUNCTION__,__LINE__, "Verify video playback using proc entry");
    }

    terminatePipeline(data.playbin);
}
GST_END_TEST;

/********************************************************************************************************************
 * Purpose      : Test to do verify playback of only audio streams
 * Parameters   : Playback URL
 ********************************************************************************************************************/
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

    terminatePipeline(data.playbin);
}
GST_END_TEST;

/********************************************************************************************************************************
 * Purpose      : Test to do restrict the connection speed and verify if playbin is able to adapt and play at expected resolution
 * Parameters   : Playback URL
 ********************************************************************************************************************************/
GST_START_TEST (test_video_bitrate)
{
    MessageHandlerData data;

    SetupPipeline(&data,false);

    assert_failure (data.playbin,connection_speed != 0,"connection speed is not given");
    g_object_set (data.playbin, "connection_speed", connection_speed, NULL);
    printf("\nConnection Speed: %" G_GUINT64_FORMAT "\n", connection_speed);

    if (log_enabled)
    {
        fprintf (file,"\n#################################\n");
        fprintf (file,"# Setting Pipeline to PLAYING State\n");
        fprintf (file,"###################################\n");
    }
    GST_FIXME( "Setting to Playing State\n");
    assert_failure (data.playbin, gst_element_set_state (data.playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "Failed to set to PLAYING state",
		    __FUNCTION__,__LINE__, "Set pipeline to PLAYING state");
    GST_FIXME( "Set to Playing State\n");
    WaitForOperation;

    printf("\nPlaying the pipeline to reach maximum connection_speed\n");

    if (checkNewPlay)
        PlaybackValidation(&data,play_timeout);
    else
        PlaySeconds(data.playbin,play_timeout);

    printf("\nValidating video height");
    g_object_get (data.westerosSink.sink, "video-height", &height, NULL);
    g_object_get (data.westerosSink.sink, "video-width", &width, NULL);
    printf("\nVideo height = %d\nVideo width = %d", height, width);
    printf("\nExpected Video height = %d",expected_height);
    char resolution_value_string[130];
    sprintf(resolution_value_string,"\nPipeline is not playing at expected resolution\nObtained video-height as %d | Expected video-height as %d",height,expected_height);
    assert_failure (data.playbin, expected_height == height ,resolution_value_string, __FUNCTION__,__LINE__, "Verify Video is playing at expected resolution");
    printf("\nSUCCESS : Pipeline restricted to expected resolution for full duration of playback\n");

    terminatePipeline(data.playbin);
}
GST_END_TEST;

/********************************************************************************************************************
 * Purpose      : Test to do check whether EOS message is received immediately when seeked to EOS
 * Parameters   : Playback URL
 ********************************************************************************************************************/
GST_START_TEST (test_seek_EOS)
{
    GstMessage *message;
    use_westerossink_fps = false;
    bool seeked = false;
    GstBus *bus;
    bool received_EOS = false;
    MessageHandlerData data;
    int operation_timeout = 5;
    GstState cur_state;
    GstStateChangeReturn state_change;


    SetupPipeline(&data);

    if (seek_with_pause)
    {
	assert_failure (data.playbin, gst_element_set_state (data.playbin, GST_STATE_PAUSED) !=  GST_STATE_CHANGE_FAILURE, "Failed to set to PAUSED state",
                    __FUNCTION__,__LINE__, "Set pipeline to PAUSED state");
        do{
             //Waiting for state change
             state_change = gst_element_get_state (data.playbin, &cur_state, NULL, 10000000);
        } while (state_change == GST_STATE_CHANGE_ASYNC);
        assert_failure (data.playbin, gst_element_get_state (data.playbin, &cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS, "Failed to get pipeline state",
			__FUNCTION__,__LINE__, "Obtain pipeline state");
        assert_failure (data.playbin, cur_state == GST_STATE_PAUSED, "Failed to set pipeline to PAUSED state", __FUNCTION__,__LINE__,  "Verify if pipeline is set to PAUSED state");
        printf("\nSTATE CHANGED FROM PLAYING TO PAUSED\n");
    }

    gst_element_get_state (data.playbin, &cur_state, NULL, (GST_SECOND));
    bus = gst_element_get_bus (data.playbin);
    assert_failure (data.playbin,bus != NULL,"Failed to get bus", __FUNCTION__,__LINE__,  "Get Playbin bus");

    /* Check if seek position was provided via command line */
    assert_failure (data.playbin, seek_position != -1, 
                    "Seek position not provided. Use: seekPosition=<seconds> (e.g., seekPosition=39)", 
                    __FUNCTION__, __LINE__, "Verify seek position is provided via command line");

    /* Use provided seek position */
    data.seekPosition = seek_position;
    gint64 duration = (play_timeout + 30) * GST_SECOND;  /* Reference duration for logging */

    printf("\n[test_seek_EOS] Using seek position from command line: %" G_GINT64_FORMAT " seconds\n", 
           GST_TIME_AS_SECONDS(data.seekPosition));
    printf("[test_seek_EOS] Reference duration: %" G_GINT64_FORMAT " seconds (play_timeout=%d + 30s)\n", 
           GST_TIME_AS_SECONDS(duration), play_timeout);

    printf ("\nSeeking to position: %" G_GINT64_FORMAT " nanoseconds\n", data.seekPosition);
    printf ("\nSEEK POSITION is set as %" G_GINT64_FORMAT "\n", data.seekPosition);

    //Seek call
    assert_failure (data.playbin,gst_element_seek (data.playbin, NORMAL_PLAYBACK_RATE, GST_FORMAT_TIME,
                                   GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, data.seekPosition,
                                   GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE), "Failed to seek",
		    __FUNCTION__,__LINE__,"Seek to EOS position");

    start = std::chrono::steady_clock::now();
    while((!received_EOS) || (!seeked))
    {
       /* Check if EOS was received - if so, test passes regardless of seek position */
       message = gst_bus_pop_filtered (bus,(GstMessageType)GST_MESSAGE_EOS);
       if (message != NULL)
       {
           if (GST_MESSAGE_EOS == GST_MESSAGE_TYPE(message))
           {
               received_EOS = true;
               printf("[test_seek_EOS] EOS message received during seek\n");
           }
       }
       
       /* If EOS received, we successfully reached end of stream - test passes */
       if (received_EOS)
       {
           printf("[test_seek_EOS] EOS received - test objective achieved\n");
           break;
       }

       //Check if seek happened
       assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &(data.currentPosition)),
                                         "Failed to query the current playback position", __FUNCTION__,__LINE__, "Query playback duration");
       //Added GST_SECOND buffer time between currentPosition and seekPosition
       if (abs( data.currentPosition - data.seekPosition) <= (GST_SECOND))
       {
           if (!seeked)  /* Only print once when seek is first detected */
           {
               printf("[test_seek_EOS] Seek completed: current position %" G_GINT64_FORMAT " within 1s of target %" G_GINT64_FORMAT "\n",
                      data.currentPosition, data.seekPosition);
           }
           seeked = TRUE;
	   //Reset to playing state
	   if (seek_with_pause)
           {
	       assert_failure (data.playbin, gst_element_set_state (data.playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "Failed to set to PLAYING state",
                    __FUNCTION__,__LINE__, "Set pipeline to PLAYING state");
               do{
                   //Waiting for state change
                   state_change = gst_element_get_state (data.playbin, &cur_state, NULL, 10000000);
               } while (state_change == GST_STATE_CHANGE_ASYNC);
               assert_failure (data.playbin, gst_element_get_state (data.playbin, &cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS,"Failed to get pipeline state",
			       __FUNCTION__,__LINE__, "Obtain pipeline state");
               assert_failure (data.playbin, cur_state == GST_STATE_PLAYING, "Failed to set pipeline to PLAYING state", __FUNCTION__,__LINE__, "Verify pipeline is set to PLAYING state");
               printf("\nSTATE CHANGED FROM PAUSED TO PLAYING\n");
	       seek_with_pause = false;
	   }
       }
       
       if (std::chrono::steady_clock::now() - start > std::chrono::seconds(operation_timeout))
       {
           printf("[test_seek_EOS] Timeout waiting for seek and/or EOS after %d seconds\n", operation_timeout);
           break;
       }
    }
    /* TEST SUCCESS CRITERIA: EOS message MUST be received when seeking to end of stream */
    assert_failure (data.playbin, received_EOS == true, 
                    "FAILED: No EOS message received within timeout. EOS reception is mandatory for test success.", 
                    __FUNCTION__, __LINE__, "Verify EOS message is received when seeking to end of stream");

    printf ("\n[test_seek_EOS] TEST PASSED: EOS message received successfully\n");
    printf ("[test_seek_EOS] Current playback position: %" G_GINT64_FORMAT " nanoseconds\n", data.currentPosition);
    printf ("[test_seek_EOS] Seek position was: %" G_GINT64_FORMAT " nanoseconds\n", data.seekPosition);

    printf ("\nEOS Received with pipeline in %s state\n",gst_element_state_get_name(cur_state));

    terminatePipeline(data.playbin);
}
GST_END_TEST;

/********************************************************************************************************************
 * Purpose      : Test to do check the underflow handling capability via appsrc
 * Parameters   : Playback URL
 ********************************************************************************************************************/
GST_START_TEST (test_appsrc_underflow)
{

    MessageHandlerData data;
    TdkFeedData feed = {0};
    bufferUnderflowTest = true;
    play_timeout = 35;
    force_appsrc = true;
    SetupPipeline(&data,false);

    assert_failure (data.playbin, gst_element_set_state (data.playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "Failed to set pipeline to PLAYING state",
		    __FUNCTION__, __LINE__, "Set pipeline to PLAYING state");

    /* Check if URL is a container file and demux if needed */
    if (tdkIsContainer(m_play_url))
    {
        printf("\nDetected container file, demuxing before appsrc feeding\n");
        
        /* Initialize queues for demux to populate */
        feed.video.app.auQueue = g_queue_new();
        feed.audio.app.auQueue = g_queue_new();
        
        if (tdkDemuxContainer(m_play_url, &feed))
        {
            guint vc = g_queue_get_length(feed.video.app.auQueue);
            guint ac = g_queue_get_length(feed.audio.app.auQueue);
            printf("\nSuccessfully demuxed container: video AUs=%u, audio AUs=%u\n", vc, ac);
            
            /* Select appropriate queue based on test mode */
            if (video_underflow_test && feed.video.app.demuxed)
            {
                printf("\nVideo underflow test: feeding video queue to appsrc\n");
                data.app.auQueue = feed.video.app.auQueue;
                data.app.demuxed_cumulative_duration_ns = 0;  /* Reset duration counter */
                /* Set video caps if available */
                if (data.app.appsrc && feed.videoCaps)
                    g_object_set(data.app.appsrc, "caps", feed.videoCaps, NULL);
            }
            else if (audio_underflow_test && feed.audio.app.demuxed)
            {
                printf("\nAudio underflow test: feeding audio queue to appsrc\n");
                data.app.auQueue = feed.audio.app.auQueue;
                data.app.demuxed_cumulative_duration_ns = 0;  /* Reset duration counter */
                /* Set audio caps if available */
                if (data.app.appsrc && feed.audioCaps)
                    g_object_set(data.app.appsrc, "caps", feed.audioCaps, NULL);
            }
            else
            {
                printf("\nNo matching test mode or demuxed stream, falling back to raw file feeding\n");
                /* Free unused queues */
                if (feed.video.app.auQueue)
                    g_queue_free_full(feed.video.app.auQueue, (GDestroyNotify)gst_buffer_unref);
                if (feed.audio.app.auQueue)
                    g_queue_free_full(feed.audio.app.auQueue, (GDestroyNotify)gst_buffer_unref);
                data.app.offset = 0;
            }
            
            /* Enable demuxed queue mode if queue was selected */
            if (data.app.auQueue)
                data.app.demuxed = true;
        }
        else
        {
            printf("\nDemux failed, falling back to raw file feeding\n");
            /* Free allocated queues on demux failure */
            if (feed.video.app.auQueue)
                g_queue_free(feed.video.app.auQueue);
            if (feed.audio.app.auQueue)
                g_queue_free(feed.audio.app.auQueue);
            data.app.offset = 0;
        }
    }
    else
    {
        /* Non-container file: use standard appsrc offset-based feeding */
        data.app.offset = 0;
    }
    
    printf("\nSetting data.terminate as false\n");
    data.terminate = false;
    while(read_data(&data));
    WaitForOperation;

    PlaybackValidation(&data,play_timeout);

    if (video_underflow_test)
	 assert_failure (data.playbin,videoUnderflowReceived == true, "Failed to receive video buffer underflow signal",
			 __FUNCTION__, __LINE__, "Verify if video buffer underflow signal is received");
    if (audio_underflow_test)
	 assert_failure (data.playbin,audio_underflow_received_global == true, "Failed to receive audio buffer underflow signal",
			 __FUNCTION__, __LINE__, "Verify if audio buffer underflow signal is received");
    if (checkSignalTest)
         goto EXIT;

    BYTES_THRESHOLD *= 2;
    printf ("\nUpdating bytestThreshold to %lld",BYTES_THRESHOLD);
    if (log_enabled)
	 fprintf(file, "\nUpdating bytestThreshold to %lld",BYTES_THRESHOLD);
    printf ("\nPushing buffers with updated bytesThreshold\n");
    videoUnderflowReceived = false;
    audio_underflow_received_global = false;
    
    /* Reset duration counter to push remaining buffers in phase 2
     * DO NOT reset segment_event_sent flag - segment should only be sent once
     * Resending segment in phase 2 would reset playback position back to 0
     */
    if (data.app.demuxed)
    {
        data.app.demuxed_cumulative_duration_ns = 0;
        /* Keep segment_event_sent = TRUE to prevent resending segment in phase 2 */
        printf("\nReset duration counter for phase 2 buffer push (keeping existing segment)\n");
        if (log_enabled)
            fprintf(file, "\nReset duration counter for phase 2 buffer push (keeping existing segment)\n");
    }
    
    data.terminate = true;
    ignoreEOS = false;
    while(read_data(&data));

    PlaybackValidation(&data,play_timeout);

EXIT:
    terminatePipeline(data.playbin);
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
    if (log_enabled)
	 fprintf(file, "\nBit_Depth_Got = %d\n", Bit_depth_got);
    
    assert_failure (data.playbin,(bit_depth == Bit_depth_got), "Expected Bit Depth not received", __FUNCTION__, __LINE__, "Verify Bit Depth Received");

    terminatePipeline(data.playbin);

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

    terminatePipeline(data.playbin);
}
GST_END_TEST;

bool compare_resolutions(const pair<unsigned int, unsigned int>& res1, const pair<unsigned int, unsigned int>& res2) {
    if (res1.second == res2.second) 
    {
	    if (Switch_up)
	    {
	        return res1.first < res2.first;  // Sort by width if heights are equal
	    }
	    else
	    {
		return res1.first > res2.first;
	    }
    }
    if (Switch_up)
    {
        return res1.second < res2.second;  // Sort by height otherwise
    }
    else
    {
        return res1.first > res2.first;
    }
}

GST_START_TEST(test_bitrate_switch)
{
    // Setup the pipeline
    MessageHandlerData data;
    SetupPipeline(&data);

    // Vector to store the resolutions (height, bandwidth) parsed from the MPD
    vector<pair<unsigned int, unsigned int>> resolutions;

    // Call the parse_mpd function, which will populate the resolutions vector
    assert_failure(data.playbin, parse_mpd(m_play_url, resolutions), "\nError in parsing MPD file");  // m_play_url is passed here

    // Check if any resolutions were parsed
    if (resolutions.empty()) {
        printf("Error: No resolutions parsed from MPD file\n");
        return;

    }

    unsigned int height_before = 0;
    // Loop through the parsed resolutions (height, bandwidth pairs)
    sort(resolutions.begin(), resolutions.end(), compare_resolutions);
    int i=1;
    for (const auto& res : resolutions) {
	unsigned int height_str = res.first;
	unsigned int bandwidth_str = res.second;

        unsigned int bandwidth = bandwidth_str;

        // Print the parsed height and bandwidth
        printf("\nHeight: %u, Bandwidth: %u\n", height_str, bandwidth_str);
	if (log_enabled)
	    fprintf(file, "\nHeight: %u, Bandwidth: %u\n", height_str, bandwidth_str);
	if (height_before > 0)
        {
            printf("Height before flush: %u, Bandwidth to set: %u\n", height_before, bandwidth);
	    if (log_enabled)
		fprintf(file, "Height before flush: %u, Bandwidth to set: %u\n", height_before, bandwidth);
	}
	height_before = height_str;

        // Set the current bandwidth in the data structure
        data.current_bandwidth = bandwidth;

        // Simulate flush operation
	g_object_set(G_OBJECT(dash_demux), "max-bitrate", data.current_bandwidth, NULL);
	if (log_enabled)
	    fprintf(file, "Setting max-bitrate to %d", data.current_bandwidth);
        flushPipeline(&data);
	if (checkNewPlay)
            PlaybackValidation(&data,play_timeout);
        else
            PlaySeconds(data.playbin,play_timeout);
	

        // Assume height remains unchanged after flush for validation
	unsigned int height_after;
	g_object_get (data.westerosSink.sink, "video-height", &height_after, NULL);

        // Check if the height is the same before and after flush
        if (height_before == height_after) 
	{
            printf("SUCCESS: Height remained the same after flush: %u\n", height_after);
        }
       	else 
	{
	    printf("FAILURE: Height changed after flush. Before: %u, After: %u\n", height_before, height_after);
	    assert_failure (data.playbin, false, "Resolution is not obtained as expected",__FUNCTION__, __LINE__, "Verify if expected resolution is played");
        }
	i++;
	if (i > resolution_count)
	    break;
	
    }

    terminatePipeline(data.playbin);
}
GST_END_TEST;


GST_START_TEST (test_video_PTS_sync)
{
    GstMessage *message;
    GstBus *bus;
    MessageHandlerData data;
    enable_last_sample = true;
    GstSample *sample = NULL;
    GstBuffer *buffer = NULL;
    GstStateChangeReturn state_change;
    int timeout = 5;

    SetupPipeline(&data,false);


    assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE ,"Failed to set to PLAYING state",
		    __FUNCTION__,__LINE__,"Set Pipeline to PLAYING state");
    if (log_enabled)
	    fprintf (file,"\nPipeline is setting to Playing state\n");


    //WaitForOperation;

    gint64 Current_Position = 0;
    do
    {
       state_change = gst_element_get_state (data.playbin, &(data.cur_state), NULL, 10000000);
    } while (state_change == GST_STATE_CHANGE_ASYNC);
    printf ("\n\n\nPipeline set to : %s  state \n\n\n", gst_element_state_get_name(data.cur_state));

    play_timeout = play_timeout - 5;

    if (checkNewPlay)
        PlaybackValidation(&data,play_timeout);
    else
        PlaySeconds(data.playbin,play_timeout);

    data.eosDetected = false;
    bus = gst_element_get_bus (data.playbin);

    start = std::chrono::steady_clock::now();
    do
    {
        message = gst_bus_pop_filtered (bus,(GstMessageType)GST_MESSAGE_EOS);
        if (message != NULL)
        {
            if (GST_MESSAGE_EOS == GST_MESSAGE_TYPE(message))
            {
		if (log_enabled)
		    fprintf(file, "\nEOS Received:Exiting\n");
                data.eosDetected = true;
		assert_failure (data.playbin,gst_element_query_position (data.playbin, GST_FORMAT_TIME, &Current_Position), "Failed to query the current playback position",
		    __FUNCTION__,__LINE__,"Query playback position");
                //gint64 Current_Position = 0;
                Current_Position = (data.currentPosition);
                printf ("\n CURRENT POSITION : %lld\n", Current_Position);

		if (std::chrono::steady_clock::now() - start > std::chrono::seconds(timeout))
                break;
            }
        }
    }while (!data.eosDetected);

    gst_object_unref (bus);

    assert_failure (data.playbin,data.eosDetected == true, "Failed to recieve EOS message", __FUNCTION__,__LINE__, "Verify EOS message received");

    printf ("EOS Received\n");
    if (log_enabled)
	    fprintf (file,"\nFailed to recieve the EOS maessage\n");

    assert_failure (data.playbin,gst_element_set_state (data.playbin, GST_STATE_PAUSED) !=  GST_STATE_CHANGE_FAILURE, "Failed to set to PAUSED state",
		    __FUNCTION__,__LINE__,"Set Pipeline to PAUSED state");
    if (log_enabled)
            fprintf (file,"\nPipeline setting to paused state \n");


    //  Fetch Video PTS from last-sample
    gint64 video_pts_last_sample = 0;
    g_object_get(data.westerosSink.sink, "last-sample", &sample, NULL);
    if (sample)
    {
        buffer = gst_sample_get_buffer(sample);
        if (buffer)
            video_pts_last_sample = GST_BUFFER_PTS(buffer);
        printf ("\n Video PTS from last sample : %lld\n", video_pts_last_sample);
        gst_sample_unref(sample);
    }

    gint64 PTS_Valid =0;

    PTS_Valid = ((Current_Position/GST_SECOND) - (video_pts_last_sample/GST_SECOND));

    printf ("\nDifference between PTS Value and Playback position : %lld \n", PTS_Valid);

    assert_failure (data.playbin, PTS_Valid < GST_SECOND, "PTS value from the last-sample is not alligned with the playback position", 
		    __FUNCTION__,__LINE__,"PTS value from the last-sample is not alligned with the playback position");
    if (log_enabled)
            fprintf (file,"\nPTS value from the last-sample is not alligned with the playback position\n");



    terminatePipeline(data.playbin);
}
GST_END_TEST;

/********************************************************************************************************************
Purpose:               Test appsrc channel change - single appsrc with container demuxing
                       Loads first channel, plays for specified timeout, transitions to NULL,
                       loads second channel, plays for specified timeout
*********************************************************************************************************************/
GST_START_TEST (test_appsrc_channel_change)
{
    MessageHandlerData data;
    TdkFeedData feed;
    GstState cur_state;

    printf("\n[appsrc-cc] test_appsrc_channel_change: Starting single-appsrc channel change test\n");
    printf("[appsrc-cc] Stream mode: %s, URLs: %s -> %s\n", 
           only_audio ? "audio" : "video", m_play_url, channel_url);

    /* Initialize feed structure */
    memset(&feed, 0, sizeof(feed));
    feed.video.app.auQueue = g_queue_new();
    feed.audio.app.auQueue = g_queue_new();
    feed.video.app.segment_event_sent = FALSE;
    feed.audio.app.segment_event_sent = FALSE;

    /* Setup pipeline for single appsrc */
    SetupPipeline(&data, false);

    /* Load first channel */
    printf("\n[appsrc-cc] Loading first channel: %s\n", m_play_url);
    if (!tdkIsContainer(m_play_url))
    {
        printf("[appsrc-cc] ERROR: First channel is not a container file\n");
        fail_unless(false, "First channel must be a container file for appsrc channel change test");
        gst_object_unref(data.playbin);
        return;
    }

    if (!tdkDemuxContainer(m_play_url, &feed))
    {
        printf("[appsrc-cc] ERROR: Failed to demux first channel\n");
        fail_unless(false, "Failed to demux first channel");
        if (feed.video.app.auQueue != NULL)
            g_queue_free_full(feed.video.app.auQueue, (GDestroyNotify)gst_buffer_unref);
        if (feed.audio.app.auQueue != NULL)
            g_queue_free_full(feed.audio.app.auQueue, (GDestroyNotify)gst_buffer_unref);
        gst_object_unref(data.playbin);
        return;
    }

    printf("[appsrc-cc] Successfully demuxed first channel\n");

    /* Connect source-setup callback BEFORE setting URI to avoid race condition */
    g_signal_connect(data.playbin, "source-setup", G_CALLBACK(tdkSourceSetup), &feed);

    /* Set playbin to use appsrc:// URI */
    g_object_set(data.playbin, "uri", "tdk://", NULL);

    /* Set to PLAYING state - source-setup callback will be triggered */
    assert_failure(data.playbin, gst_element_set_state(data.playbin, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE,
            "Failed to set pipeline to playing state for first channel", __FUNCTION__, __LINE__, 
            "Setting pipeline to PLAYING for first channel");

    /* Wait for source-setup to obtain appsrc element */
    MilliSleep(200);

    /* Push buffers from first channel */
    printf("\n[appsrc-cc] Pushing first channel buffers\n");
    data.terminate = false;
    if (!only_audio)
        while(read_data(&feed.video));
    if (!only_video)
        while(read_data(&feed.audio));

    printf("[appsrc-cc] Finished pushing first channel data\n");

    /* Validate playback of first channel using PlaybackValidation */
    printf("\n[appsrc-cc] Validating first channel playback (timeout: %d seconds)\n", play_timeout);
    PlaybackValidation(&data, play_timeout);

    printf("[appsrc-cc] First channel playback validation completed\n");

    /* Transition to NULL state before loading second channel */
    printf("\n[appsrc-cc] Transitioning pipeline to NULL state\n");
    assert_failure(data.playbin, gst_element_set_state(data.playbin, GST_STATE_NULL) != GST_STATE_CHANGE_FAILURE,
            "Failed to set pipeline to NULL state", __FUNCTION__, __LINE__, 
            "Setting pipeline to NULL state");

    gst_element_get_state(data.playbin, &cur_state, NULL, 5 * GST_SECOND);
    assert_failure(data.playbin, cur_state == GST_STATE_NULL, 
            "Pipeline did not reach NULL state",  __FUNCTION__, __LINE__, 
            "Verifying pipeline is in NULL state");

    printf("[appsrc-cc] Pipeline transitioned to NULL successfully\n");

    /* Clear first channel feed */
    printf("\n[appsrc-cc] Clearing first channel feed\n");
    if (feed.video.app.auQueue != NULL)
    {
        g_queue_free_full(feed.video.app.auQueue, (GDestroyNotify)gst_buffer_unref);
        feed.video.app.auQueue = NULL;
    }
    if (feed.audio.app.auQueue != NULL)
    {
        g_queue_free_full(feed.audio.app.auQueue, (GDestroyNotify)gst_buffer_unref);
        feed.audio.app.auQueue = NULL;
    }
    if (feed.videoCaps != NULL)
    {
        gst_caps_unref(feed.videoCaps);
        feed.videoCaps = NULL;
    }
    if (feed.audioCaps != NULL)
    {
        gst_caps_unref(feed.audioCaps);
        feed.audioCaps = NULL;
    }

    /* Load second channel */
    printf("\n[appsrc-cc] Loading second channel: %s\n", channel_url);
    
    /* Disconnect old signal handler and reset feed structure */
    g_signal_handlers_disconnect_by_func(data.playbin, (gpointer)tdkSourceSetup, &feed);
    
    memset(&feed, 0, sizeof(feed));
    feed.video.app.auQueue = g_queue_new();
    feed.audio.app.auQueue = g_queue_new();
    feed.video.app.segment_event_sent = FALSE;
    feed.audio.app.segment_event_sent = FALSE;
    feed.video.app.demuxed_cumulative_duration_ns = 0;
    feed.audio.app.demuxed_cumulative_duration_ns = 0;

    if (!tdkIsContainer(channel_url))
    {
        printf("[appsrc-cc] ERROR: Second channel is not a container file\n");
        fail_unless(false, "Second channel must be a container file for appsrc channel change test");
        if (feed.video.app.auQueue != NULL)
            g_queue_free_full(feed.video.app.auQueue, (GDestroyNotify)gst_buffer_unref);
        if (feed.audio.app.auQueue != NULL)
            g_queue_free_full(feed.audio.app.auQueue, (GDestroyNotify)gst_buffer_unref);
        gst_object_unref(data.playbin);
        return;
    }

    if (!tdkDemuxContainer(channel_url, &feed))
    {
        printf("[appsrc-cc] ERROR: Failed to demux second channel\n");
        fail_unless(false, "Failed to demux second channel");
        if (feed.video.app.auQueue != NULL)
            g_queue_free_full(feed.video.app.auQueue, (GDestroyNotify)gst_buffer_unref);
        if (feed.audio.app.auQueue != NULL)
            g_queue_free_full(feed.audio.app.auQueue, (GDestroyNotify)gst_buffer_unref);
        gst_object_unref(data.playbin);
        return;
    }

    printf("[appsrc-cc] Successfully demuxed second channel\n");

    /* Reset data.app fields before loading second channel */
    data.app.demuxed_cumulative_duration_ns = 0;
    data.app.segment_event_sent = FALSE;
    data.streamStart = FALSE;
    total_bytes_fed = 0;

    /* Connect source-setup callback BEFORE setting URI for second channel */
    g_signal_connect(data.playbin, "source-setup", G_CALLBACK(tdkSourceSetup), &feed);

    /* Set playbin to use appsrc:// URI for second channel */
    g_object_set(data.playbin, "uri", "tdk://", NULL);

    /* Set to PLAYING state */
    assert_failure(data.playbin, gst_element_set_state(data.playbin, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE,
            "Failed to set pipeline to playing state for second channel", __FUNCTION__, __LINE__, 
            "Setting pipeline to PLAYING for second channel");

    /* Wait for source-setup to obtain appsrc element */
    MilliSleep(200);

    /* Push buffers from second channel */
    printf("\n[appsrc-cc] Pushing second channel buffers\n");
    data.terminate = false;
    data.pipelineInitiation = TRUE;  /* Reset for second channel validation */
    if (!only_audio)
        while(read_data(&feed.video));
    if (!only_video)
        while(read_data(&feed.audio));

    printf("[appsrc-cc] Finished pushing second channel data\n");

    /* Set pipeline initiation flag for second channel validation */
    data.pipelineInitiation = TRUE;

    /* Validate playback of second channel using PlaybackValidation */
    printf("\n[appsrc-cc] Validating second channel playback (timeout: %d seconds)\n", play_timeout);
    PlaybackValidation(&data, play_timeout);

    printf("[appsrc-cc] Second channel playback validation completed\n");
    printf("[appsrc-cc] test_appsrc_channel_change PASSED\n");

    /* Cleanup */
    if (feed.video.app.auQueue != NULL)
        g_queue_free_full(feed.video.app.auQueue, (GDestroyNotify)gst_buffer_unref);
    if (feed.audio.app.auQueue != NULL)
        g_queue_free_full(feed.audio.app.auQueue, (GDestroyNotify)gst_buffer_unref);
    if (feed.videoCaps != NULL)
        gst_caps_unref(feed.videoCaps);
    if (feed.audioCaps != NULL)
        gst_caps_unref(feed.audioCaps);

    terminatePipeline(data.playbin);
}
GST_END_TEST;




static Suite *
media_pipeline_suite (void)
{
    file = fopen(LOG_FILE, "a");
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

    if (tcname[0] != '\0')
        current_test_name = tcname;

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
    else if (strcmp ("test_appsrc_channel_change", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_appsrc_channel_change);
       GST_INFO ("tc test_appsrc_channel_change run successfull\n");
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
       use_multiappsrc = true;
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
    else if (strcmp ("test_bitrate_switch_up", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_bitrate_switch);
       Switch_up = true;
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_bitrate_switch_down", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_bitrate_switch);
       Switch_up = false;
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
    else if (strcmp ("test_seek_EOS", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_seek_EOS);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_seek_EOS_with_pause", tcname) == 0)
    {
       seek_with_pause = true;
       tcase_add_test (tc_chain, test_seek_EOS);
       GST_INFO ("tc %s run successfull\n", tcname);
       GST_INFO ("SUCCESS\n");
    }
    else if (strcmp ("test_video_PTS_sync", tcname) == 0)
    {
       tcase_add_test (tc_chain, test_video_PTS_sync);
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
	    if (log_enabled)
                fprintf(file, "Error setting environment variable: %s\n", varName);
        }
	else
	{
            printf("Set environment variable: %s=%s\n", varName, varValue);
	    if (log_enabled)
                fprintf(file, "Set environment variable: %s=%s\n", varName, varValue);
        }
    }
}

/********************************************************************************************************************
 * Purpose      : To set environment file path
 ********************************************************************************************************************/
std::string GetTDKEnvPath() {
    // Fetch the environment variable TDK_ENV_PATH
    const char* env_path = std::getenv("TDK_ENV_PATH");
    // If TDK_ENV_PATH is not set, default to /opt/TDK
    if (env_path == nullptr) {
        printf("\nEnvironment file set as /opt/TDK/TDK.env\n");
        return "/opt/TDK/TDK.env";
    }
    printf("\nEnvironment file set as %s/TDK.env\n",env_path);
    // Return the value of TDK_ENV_PATH
    return std::string(env_path) + "/TDK.env";
}

void handle_LDPRELOAD(const std::string &inputStr);

/********************************************************************************************************************
 * Purpose      : To read from ENV_FILE and set the corresponding environmental variables
 ********************************************************************************************************************/
int setVariables()
{
    FILE* inputFile = fopen(GetTDKEnvPath().c_str(), "r");
    if (inputFile)
    {
        char line[256];
        while (fgets(line, sizeof(line), inputFile))
        {
             line[strcspn(line, "\n")] = '\0';
	     if (std::string(line).find("LD_PRELOAD") != std::string::npos)
	     {
	         char *varName = line + 7;
                 printf("Handling : %s",varName);
                 handle_LDPRELOAD(std::string(line));
		 continue;
             }
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
	 printf ("\nUnable to open %s file for reading\n", GetTDKEnvPath().c_str());
	 return 0;
     }
}

/********************************************************************************************************************
 * Purpose      : To avoid tearing down of pipeline while handling warning level messages
 ********************************************************************************************************************/
void log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
    if (log_level & G_LOG_LEVEL_WARNING)
    {
        printf("\nWARNING : %s\n", message);
        // Ignore warnings messages and do not exit from app
        return;
    }
    if (log_level & G_LOG_LEVEL_CRITICAL)
    {
        printf("\nCRITICAL : %s\n", message);
        // Ignore critical messages and do not exit from app
        return;
    }
}

/********************************************************************************************************************
 * Purpose      : To execute command in DUT and return output
 ********************************************************************************************************************/
std::string executeCmndInDUT(std::string command,bool debug = true)
{
    if (debug)
        printf("\nExecuting command : %s",command.c_str());
    // Execute the command using po
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        printf("\npopen() failed!\n");
        return "failure";
    }
    // Read the output of the command
    std::string output;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    // Close the pipe
    if (pclose(pipe) == -1) {
        printf("\npclose() failed!\n");
        return "failure";
    }
    return std::string(output);
}

/********************************************************************************************************************
 * Purpose      : To check if westeros is rendering or not
 ********************************************************************************************************************/
std::string WesterosProcessID(bool debug = true)
{
    std::string command = "pidof westeros";
    std::string output = executeCmndInDUT(command,debug);
    // Check if output is not empty
    if (!output.empty())
    {
	if (debug)
            printf("\npidof westeros received: %s\n",output.c_str());
	return std::string(output);
    }
    else
    {
        printf("\nWesteros not running\n");
	if (log_enabled)
            fprintf(file, "\nWesteros not running\n");
	return "NIL";
    }
}

/********************************************************************************************************************
 * Purpose      : Write callback to parse json response
 ********************************************************************************************************************/
size_t WriteCallbackJson(void *contents, size_t size, size_t nmemb, std::string *output) {
    size_t totalSize = size * nmemb;
    output->append((char *)contents, totalSize);
    return totalSize;
}

/********************************************************************************************************************
 * Purpose      : Function to perform POST request and return JSON response as a string
 ********************************************************************************************************************/
std::string sendRequest(const std::string& jsonData) {
    CURL *curl = curl_easy_init();
    std::string response;
    if (curl) {
        const char* url = "http://0.0.0.0:9998/jsonrpc";
        // Set curl options
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackJson);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
	    curlError = true;
	    response = "FAILURE";
        }
        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Failed to initialize libcurl" << std::endl;
    }
    return response;  // Return the response as a string
}

/********************************************************************************************************************
 * Purpose      : Function to parse json Response
 ********************************************************************************************************************/
bool parseResult(const std::string& jsonResponse)
{
    //Check if response is FAILURE
    if (jsonResponse == "FAILURE")
	return false;
    // Parse JSON response using jsoncpp
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;
    // Use istringstream to parse the response
    std::istringstream sstream(jsonResponse);
    if (Json::parseFromStream(builder, sstream, &root, &errs))
    {
        // Validate that root is an object before accessing members
        if (!root.isObject() && !root.isNull())
        {
            printf("\nERROR: JSON response is not an object\n");
            return false;
        }
        
        // Check for "error" field in the response (safe check)
        if (root.isObject() && root.isMember("error") && !root["error"].isNull())
        {
            std::string message = root["error"]["message"].asString();
	    printf("\nERROR : %s \n",message.c_str());
            return false;
        }
        
        // Check for "result" field
        if (!root.isObject() || !root.isMember("result"))
        {
            printf("\nERROR: No result field in JSON response\n");
            return false;
        }
        
        // Handle null result (success case - e.g., createDisplay returns null result)
        if (root["result"].isNull())
        {
            printf("\nresult : \"null\" (success)\n");
            return true;
        }
        
        // Handle array result
        if (root["result"].isArray() && root["result"].size() > 0)
        {
            // Access the 'state' field in the first object in the result array (Thunder Controller status format)
            if (root["result"][0].isObject() && root["result"][0].isMember("state"))
            {
                std::string state = root["result"][0]["state"].asString();
	        printf(" %s \n",state.c_str());
                return state == "activated";
            }
            return true;  // Array result exists, treat as success
        }
        
        // Handle object result
        if (root["result"].isObject())
        {
            // Check for "state" field (status check format)
            if (root["result"].isMember("state"))
            {
                std::string state = root["result"]["state"].asString();
	        printf(" %s \n",state.c_str());
                return state == "activated";
            }
            // Check for "apps" array (checkDisplayClient format)
            else if (root["result"].isMember("apps") && root["result"]["apps"].isArray())
            {
                // RDKWindowManager format - check apps array (object format)
                for (const auto& app : root["result"]["apps"])
                {
                    if (app.isObject() && app.isMember("name") && app["name"].asString() == "test")
                    {
                        return true;
                    }
                }
                return false;
            }
            // Check for "success" field (generic success field handling)
            else if (root["result"].isMember("success"))
            {
                bool success = root["result"]["success"].asBool();
	        printf(" success : %s\n", success ? "true" : "false");
	        return success;
            }
            else
            {
                // Generic object result - assume success
                return true;
            }
        }
        
        // Handle string result (e.g., getApps returns result as string containing JSON array)
        if (root["result"].isString())
        {
            std::string resultStr = root["result"].asString();
	    if (!resultStr.empty())
	    {
                printf("apps string: %s\n", resultStr.c_str());
            }
            // Check if the string contains "test"
            return resultStr.find("\"test\"") != std::string::npos;
        }
        
        // If result exists but is none of the above, treat as success
        return true;
    }
    else
    {
        printf("\nERROR : Unable to parse JSON response: %s\n", errs.c_str());
        if (log_enabled)
            fprintf(file, "\nERROR : Unable to parse JSON response: %s\n", errs.c_str());
        return false;
    }
    return false;
}

/********************************************************************************************************************
 * Purpose      : Function to obtain RDKWindowManager plugin status
 ********************************************************************************************************************/
bool getPluginStatus()
{
    std::string jsonData = R"({"jsonrpc": "2.0", "id": 2, "method": "Controller.1.status@org.rdk.RDKWindowManager"})";
    // Call sendRequest function to get the JSON response as string
    std::string response = sendRequest(jsonData);
    if (response == "FAILURE")
	printf ("\nERROR : Unable to send request to 0.0.0.0:9998\n");
    else
	printf("\nRDKWindowManager State : ");
    return parseResult(response);
}

/********************************************************************************************************************
 * Purpose      : Function to activate RDKWindowManager plugin
 ********************************************************************************************************************/
bool activatePlugin()
{
    std::string jsonData = R"({"jsonrpc": "2.0", "id": 2, "method": "Controller.1.activate", "params": { "callsign": "org.rdk.RDKWindowManager" }})";
    // Call sendRequest function to get the JSON response as string
    std::string response = sendRequest(jsonData);
    return parseResult(response);
}

/********************************************************************************************************************
 * Purpose      : Function to verify if a display client named test is already running
 ********************************************************************************************************************/
bool checkDisplayClient()
{
    std::string jsonData = R"({
        "jsonrpc": "2.0",
        "id": 1,
        "method": "org.rdk.RDKWindowManager.1.getApps"
    })";

    // Call sendRequest function to get the JSON response as string
    std::string response = sendRequest(jsonData);
    return parseResult(response);
}

/********************************************************************************************************************
 * Purpose      : Function to create display client via RDKWindowManager plugin
 ********************************************************************************************************************/
bool createDisplayClient()
{
    std::string jsonData = R"({
        "jsonrpc": "2.0",
        "id": 1,
        "method": "org.rdk.RDKWindowManager.1.createDisplay",
        "params": {
            "clientId": "test",
            "displayName": "test",
            "displayWidth": 1920,
            "displayHeight": 1080
        }
    })";

    // Call sendRequest function to get the JSON response as string
    std::string response = sendRequest(jsonData);
    return parseResult(response);
}

/********************************************************************************************************************
 * Purpose      : Function to destroy display client via RDKWindowManager plugin
 ********************************************************************************************************************/
bool destroyDisplayClient()
{
    printf("\nDestroying display client via RDKWindowManager plugin is not implemented\n");
    return true;
}

/********************************************************************************************************************
 * Purpose      : To handle LD_PRELOAD by loading libraries using dlopen
 ********************************************************************************************************************/
void handle_LDPRELOAD(const std::string &inputStr)
{
    std::string prefix = "export LD_PRELOAD=";
    // Ensure the input string starts with the expected prefix
    if (inputStr.find(prefix) != 0) {
        std::cerr << "Invalid LD_PRELOAD string format!\n";
        return;
    }
    // Extract the actual library paths (removing "export LD_PRELOAD=")
    std::string ldPreloadStr = inputStr.substr(prefix.length());
    std::stringstream ss(ldPreloadStr);
    std::string libPath;
    // Split by ':' and load each shared library
    while (std::getline(ss, libPath, ':'))
    {
        void *handle = dlopen(libPath.c_str(), RTLD_LAZY);
        if (!handle)
	{
            std::cerr << "Failed to load: " << libPath << " | Error: " << dlerror() << "\n";
	    if (log_enabled)
		fprintf(file, "Failed to load: %s ", libPath.c_str());
        }
	else
	{
            printf("\nLoaded: %s",libPath.c_str());
	    if (log_enabled)
	        fprintf(file, "\nLoaded: %s\n",libPath.c_str());
            handles.push_back(handle);
        }
    }
    printf("\n");
}

/********************************************************************************************************************
 * Purpose      : To close libraries opened via dlclose
 ********************************************************************************************************************/
void closeLibs()
{
    // Close libraries before exiting
    if (!handles.empty())
    {
	printf("Closing libs");
	if (log_enabled)
	    fprintf(file, "\nClosing libs");
    }
    for (void *handle : handles)
    {
        if (handle)
	{
            dlclose(handle);
        }
    }
    printf("\n");
}


/********************************************************************************************************************
 * Purpose      : To start westeros by executing commands from TDK.env
 ********************************************************************************************************************/
bool startWesteros()
{
    std::ifstream infile(GetTDKEnvPath());
    std::string command = "source " + GetTDKEnvPath();
    if (!infile) {
        printf("\nCould not open the file!\n");
	if (log_enabled)
            fprintf(file, "\nCould not open the file %s!\n", GetTDKEnvPath().c_str());
        return false;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        printf("\nFork failed!\n");
	return false;
    }

    if (pid == 0)
    {
        // In the child process
	// Execute the command
	// Use "/bin/sh -c" to run the command in a shell
	printf("\nInitializing westeros\n");
	fprintf(file, "\nInitializing westeros\n");
	execl("/bin/sh", "sh", "-c", command.c_str(), (char*) nullptr);
	return true;
    }
   
    //Return true from parent process
    return true;
}

void printTest(int argc, char **argv)
{
    if (log_enabled)
    {
	fprintf(file,"\n%s",std::string(188,'#').c_str());
        fprintf (file,"\n# TestCase Name: %s \n", tcname);
        fprintf (file,"# Command invoked -> \n");
        for (int i=0; i <argc; i++)
        {
            fprintf (file," %s ", argv[i]);
        }
	fprintf(file,"\n%s\n\n",std::string(188,'#').c_str());
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
    bool startWesterosConfig = false;
    bool createDisplayConfig = false;
    bool displayCreated = false;
    file = fopen(LOG_FILE, "w");

    gst_check_init (&argc, &argv);
    if (argc >= 1)
    {
        strcpy (tcname, argv[1]);
    }

    printTest(argc, argv);
    
    /*
     * Get TDK path
     */
    if (getenv ("TDK_PATH") != NULL)
    {
    	strcpy (TDK_PATH, getenv ("TDK_PATH"));
    }
    else
    {
	if (access(GetTDKEnvPath().c_str(), F_OK) == 0)
	{
	     if (log_enabled)
             {
                 fprintf(file , "\n##################################################################\n");
                 fprintf(file ,   "####################  Setting up Pre-requisites ##################\n");
                 fprintf(file ,   "##################################################################\n");
             }
	     if (!(setVariables()))
		goto exit;
	     strcpy (TDK_PATH, getenv ("TDK_PATH"));
	}
	else
	{
	    GST_ERROR ("Environment variable TDK_PATH should be set!!!!");
	    printf ("Environment variable TDK_PATH is not set!!!!\n");
	    printf ("Environment variables can be set in /opt/TDK/TDK.env\n");
	    goto exit;
	}
    }


    if (getenv ("AUDIO_PTS_CHECK") != NULL)
    {
        AudioPTSCheckAvailable = true;
    }
    if (argc == 2)
    {
    	GST_INFO ("\nArg 2: TestCase Name: %s \n", tcname);
    	printf ("\nArg 2: TestCase Name: %s \n", tcname);
    }
    else if (argc >= 3)
    {
	if ((strcmp ("test_channel_change_playback", tcname) == 0) ||
	    (strcmp ("test_appsrc_channel_change", tcname) == 0))
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
	    (strcmp ("test_seek_EOS", tcname) == 0) ||
	    (strcmp ("test_seek_EOS_with_pause", tcname) == 0) ||
            (strcmp ("test_EOS", tcname) == 0) ||
	    (strcmp ("test_audio_sampling_rate", tcname) == 0) ||
	    (strcmp ("test_only_audio", tcname) == 0) ||
	    (strcmp ("test_video_bitrate", tcname) == 0) ||
	    (strcmp ("test_color_depth", tcname) == 0) ||
	    (strcmp ("test_audio_volume", tcname) == 0) ||
	    (strcmp ("test_audio_volume_stress", tcname) == 0) ||
	    (strcmp ("test_playback_duration", tcname) == 0) ||
	    (strcmp ("test_channel_change_playback", tcname) == 0) ||
	    (strcmp ("test_appsrc_channel_change", tcname) == 0) ||
	    (strcmp ("test_audio_duration", tcname) == 0) || 
	    (strcmp ("test_bitrate_switch_down", tcname) == 0) || 
	    (strcmp ("test_video_PTS_sync", tcname) == 0) ||
	    (strcmp ("test_bitrate_switch_up", tcname) ==0))
	{
	    strcpy(m_play_url,argv[2]);

            for (; arg < argc; arg++)
            {
                if (strcmp ("checkavstatus=yes", argv[arg]) == 0)
                {
                    checkAVStatus = true;
                }
		// For Channel Change tests, timeout handling is done separately
                if ((strstr (argv[arg], "timeout=") != NULL) && 
                    (strcmp ("test_channel_change_playback", tcname) != 0) &&
                    (strcmp ("test_appsrc_channel_change", tcname) != 0))
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
		if (strstr (argv[arg], "seekPosition=") != NULL)
                {
                    strtok (argv[arg], "=");
                    gint64 seek_pos_seconds = atoi (strtok (NULL, "="));
                    seek_position = seek_pos_seconds * GST_SECOND;
                    printf("[main] Seek position set to %lld seconds (%" G_GINT64_FORMAT " nanoseconds)\n", 
                           seek_pos_seconds, seek_position);
                }
		if (strstr (argv[arg], "height=") != NULL)
                {
                    strtok (argv[arg], "=");
                    expected_height = atoi (strtok (NULL, "="));
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
		if (strcmp ("ignoreEOS", argv[arg]) == 0)
                {
                    ignoreEOS = true;
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
		if (strcmp ("validateFullPlayback=no", argv[arg]) == 0)
                {
                    checkNewPlay = false;
                }
		if (strcmp ("checkEachSecondPlayback", argv[arg]) == 0)
		{
		    checkEachSecondPlayback = true;
		}
		if (strcmp ("forceAudioSink", argv[arg]) == 0)
                {
                    forceAudioSink = true;
                }
		if ((strcmp ("test_channel_change_playback", tcname) == 0) ||
		    (strcmp ("test_appsrc_channel_change", tcname) == 0))
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
		if (strcmp ("use_multiappsrc=yes", argv[arg]) == 0)
                {
                    use_multiappsrc = true;
                }
		if (strcmp ("only_audio", argv[arg]) == 0)
                {
                    only_audio = true;
                }
		if (strstr (argv[arg], "media=") != NULL)
		{
		    strtok (argv[arg], "=");
		    char *media_type = strtok (NULL, "=");
		    if (strcmp (media_type, "audio") == 0)
		    {
		        only_audio = true;
		        only_video = false;
		        printf("\nMedia mode set to AUDIO ONLY\n");
		    }
		    else if (strcmp (media_type, "video") == 0)
		    {
		        only_video = true;
		        only_audio = false;
		        printf("\nMedia mode set to VIDEO ONLY\n");
		    }
		}
		if (strcmp ("startWesteros=yes", argv[arg]) == 0)
		{
		    startWesterosConfig = true;
		    defaultStart = false;
		}
		if (strcmp ("live_stream", argv[arg]) == 0)
                {
		    live_stream = true;
                }
		if (strcmp ("createDisplay=yes", argv[arg]) == 0)
                {
                    createDisplayConfig = true;
		    startWesterosConfig = false;
		    defaultStart = false;
                }
            }

            printf ("\nArg : TestCase Name: %s \n", tcname);
            printf ("\nArg : Playback url: %s \n", m_play_url);

	    if ((strcmp ("test_channel_change_playback", tcname) == 0) ||
	        (strcmp ("test_appsrc_channel_change", tcname) == 0))
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

    if (defaultStart)
    {
	bool RDKWindowManager_exists = fileExists("/etc/WPEFramework/plugins/RDKWindowManager.json" , true);
	if (RDKWindowManager_exists)
        {
	    printf("\nRDKWindowManager is present in device\nCreating display using RDKWindowManager -- 'test'\n");
	    if (log_enabled)
		fprintf(file, "\nRDKWindowManager is present in device\nCreating display using RDKWindowManager -- 'test'\n");
	    createDisplayConfig = true;
            startWesterosConfig = false;
        }
        else
        {
	    printf("\nRDKWindowManager is not present in device\nProceeding to create display using westeros renderer\n");
	    if (log_enabled)
		fprintf(file, "\nRDKWindowManager is not present in device\nProceeding to create display using westeros renderer\n");
	    createDisplayConfig = false;
            startWesterosConfig = true;
        }
    }

    g_log_set_handler (NULL, (GLogLevelFlags) (G_LOG_LEVEL_WARNING|G_LOG_LEVEL_CRITICAL),
      log_handler, NULL);
    g_log_set_handler ("GStreamer", (GLogLevelFlags) (G_LOG_LEVEL_WARNING|G_LOG_LEVEL_CRITICAL),
      log_handler, NULL);
    g_log_set_handler ("GLib-GObject", (GLogLevelFlags) (G_LOG_LEVEL_WARNING|G_LOG_LEVEL_CRITICAL),
      log_handler, NULL);
    g_log_set_handler ("GLib-GIO", (GLogLevelFlags) (G_LOG_LEVEL_WARNING|G_LOG_LEVEL_CRITICAL),
      log_handler, NULL);
    g_log_set_handler ("GLib", (GLogLevelFlags) (G_LOG_LEVEL_WARNING|G_LOG_LEVEL_CRITICAL),
      log_handler, NULL);

    if (createDisplayConfig)
    {
	     
	if(!getPluginStatus())
        {
	     if (curlError)
		  goto exit;
             printf("\nRDKWindowManager is deactivated\n");
             fprintf(file, "\nRDKWindowManager is deactivated\n");
             printf("Activating RDKWindowManager\n");
             fprintf(file, "Activating RDKWindowManager\n");
             if (activatePlugin())
             {
                  printf("\nActivate RDKWindowManager success\n");
                  //Wait for plugin to activate
                  sleep(5);
		  if (!getPluginStatus())
		  {
		       printf("\nERROR : Unable to activate RDKWindowManager plugin");
		       fprintf(file, "\nERROR : Unable to activate RDKWindowManager plugin");
		       goto exit;
		  }
             }
	 }
	 if (checkDisplayClient())
	 {
	     printf("\nAlready a display \"test\" is running in DUT");
	     printf("\nRe-using display");
	     if (log_enabled)
	         fprintf(file, "\nAlready a display \"test\" is running in DUT\nRe-using display\n");
	 }
	 else
	 {
	     printf("\nCreating RDKWindowManager Display");
	     if (log_enabled)
	         fprintf(file, "\nCreating RDKWindowManager Display");
             if (!createDisplayClient())
             {
	          printf("\nERROR : Unable to create Display\n");
		  if (log_enabled)
	              fprintf(file, "\nERROR : Unable to create Display\n");
                  goto exit;
             }
             else
             {
		  printf("\nSUCCESS : RDKWindowManager display created successfully\n");
		  if (log_enabled)
                      fprintf(file, "\nSUCCESS : RDKWindowManager display created successfully\n");
	          displayCreated = true;
	     }
	 }

         // Set WAYLAND_DISPLAY to "test" as display manager creates a window with displayName as "test"
         printf("\nSetting WAYLAND_DISPLAY to \"test\"");
         fprintf(file, "\nSetting WAYLAND_DISPLAY to \"test\"");
         setenv("WAYLAND_DISPLAY", "test", 1);
         if (strcmp(getenv ("WAYLAND_DISPLAY"), "test") == 0)
	 {
             printf("\nWAYLAND_DISPLAY successfully set to \"test\"\n\n");
             fprintf(file, "\nWAYLAND_DISPLAY successfully set to \"test\"\n\n");
	 }
         else
         {
             printf("\nUnable to set WAYLAND_DISPLAY to \"test\"\n\n");
             fprintf(file, "\nUnable to set WAYLAND_DISPLAY to \"test\"\n\n");
             goto exit;
         }
    }
    else if (startWesterosConfig)
    {
        /* Skip RDKWindowManager initialization when using westeros */
        printf("\nSkipping RDKWindowManager initialization - using westeros renderer\n");
        if (log_enabled)
            fprintf(file, "\nSkipping RDKWindowManager initialization - using westeros renderer\n");
    }
    else
    {
	if(!getPluginStatus())
        {
	     if (curlError)
		  goto exit;
             printf("\nRDKWindowManager is deactivated\n");
             fprintf(file, "\nRDKWindowManager is deactivated\n");
             printf("Activating RDKWindowManager\n");
             fprintf(file, "Activating RDKWindowManager\n");
             if (activatePlugin())
             {
		  printf("\nActivate RDKWindowManager success\n");
                  //Wait for plugin to activate
                  sleep(5);
                  if (!getPluginStatus())
                  {
                       printf("\nERROR : Unable to activate RDKWindowManager plugin");
                       fprintf(file, "\nERROR : Unable to activate RDKWindowManager plugin");
                       goto exit;
                  }
             }
	 }
	 if (checkDisplayClient())
	 {
	     printf("\nAlready a display \"test\" is running in DUT");
	     printf("\nRe-using display");
	     if (log_enabled)
	         fprintf(file, "\nAlready a display \"test\" is running in DUT\nRe-using display\n");
	 }
	 else
	 {
             printf("\nCreating RDKWindowManager Display");
             if (log_enabled)
                 fprintf(file, "\nCreating RDKWindowManager Display");
             if (!createDisplayClient())
             {
                  printf("\nERROR : Unable to create Display\n");
                  if (log_enabled)
                      fprintf(file, "\nERROR : Unable to create Display\n");
                  goto exit;
             }
             else
             {
                  printf("\nSUCCESS : RDKWindowManager display created successfully\n");
                  if (log_enabled)
                      fprintf(file, "\nSUCCESS : RDKWindowManager display created successfully\n");
                  displayCreated = true;
             }
	 }

         // Set WAYLAND_DISPLAY to "test" as RDKWindowManager creates a window with displayName as "test"
         printf("\nSetting WAYLAND_DISPLAY to \"test\"");
         fprintf(file, "\nSetting WAYLAND_DISPLAY to \"test\"");
         setenv("WAYLAND_DISPLAY", "test", 1);
         if (strcmp(getenv ("WAYLAND_DISPLAY"), "test") == 0)
	 {
             printf("\nWAYLAND_DISPLAY successfully set to \"test\"\n\n");
             fprintf(file, "\nWAYLAND_DISPLAY successfully set to \"test\"\n\n");
	 }
         else
         {
             printf("\nUnable to set WAYLAND_DISPLAY to \"test\"\n\n");
             fprintf(file, "\nUnable to set WAYLAND_DISPLAY to \"test\"\n\n");
             goto exit;
         }
    }
    if ((startWesterosConfig) && (WesterosProcessID() == "NIL"))
    {
	if (!startWesteros())
	    goto exit;
	Sleep(1);
        if (WesterosProcessID() == "NIL")
	{
	    printf("\nERROR : Unable to start westeros compositor\n");
	    fprintf(file, "\nERROR : Unable to start westeros compositor\n");
	    goto exit;
	}
	else
	{
	    westerosStarted = true;
	}
    }
    if (log_enabled)
    {
        fprintf(file , "\n##################################################################\n");
        fprintf(file ,   "################# Pre-requisites successfully set ################\n");
        fprintf(file ,   "##################################################################\n");
    }
    fclose(file);
    testSuite = media_pipeline_suite ();
    returnValue =  gst_check_run_suite (testSuite, "playbin_plugin_test", __FILE__);
exit:
    if (!file)
        file = fopen(LOG_FILE, "a");
    if ( ((startWesterosConfig && westerosStarted) || (createDisplayConfig && displayCreated)) && log_enabled)
    {
	fprintf(file , "\n##################################################################\n");
        fprintf(file ,   "################# Executing Post-requisites ######################\n");
        fprintf(file ,   "##################################################################\n");
    }
    if (startWesterosConfig && westerosStarted)
    {
	std::string command = "kill -9 " + WesterosProcessID(false);
	printf("Deinitializing westeros\n");
	if (log_enabled)
            fprintf(file, "Deinitializing westeros\n");
        executeCmndInDUT(command,false);
    }
    if (createDisplayConfig && displayCreated)
    {
	printf("Destroying RDKWindowManager Display");
	if (log_enabled)
            fprintf(file, "Destroying RDKWindowManager Display");
	if (!destroyDisplayClient())
	{
	    printf("ERROR: Unable to destroy RDKWindowManager display\n");
	    if (log_enabled)
		fprintf(file, "ERROR: Unable to destroy RDKWindowManager display\n");
	    //Add destroying display failure to number of failures
	    returnValue += 1;
	}
    }
    closeLibs();
    if (log_enabled)
	fprintf(file , "\n################## End of Execution ##############################\n");
    fclose(file);
    return returnValue;
}


