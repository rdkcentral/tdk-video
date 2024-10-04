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
#include <bits/stdc++.h>
#include <iterator>
#include <string>
#include <vector>
#include <cmath>
#include <time.h>
#include <ctime>
#include <sys/time.h>
#include <sstream>
#include <chrono>
extern "C"
{
#include <gst/check/gstcheck.h>
#include <gst/gst.h>
}
using namespace std;
#define RATE_SET_TIMEOUT                5
#define DEFAULT_TEST_SUITE_TIMEOUT      360
#define VIDEO_STATUS                    "/CheckVideoStatus.sh"
#define AUDIO_STATUS                    "/CheckAudioStatus.sh"
#define PLAYBIN_ELEMENT                 "playbin"
#define WESTEROS_SINK                   "westerossink"
#define BUFFER_SIZE_LONG                1024
#define BUFFER_SIZE_SHORT               264
#define NORMAL_PLAYBACK_RATE            1.0
#define FCS_MICROSECONDS                1000000
#define Sleep(RunSeconds)               start = std::chrono::high_resolution_clock::now(); \
                                        Runforseconds = RunSeconds; \
                                        while(1) { \
                                        if (std::chrono::high_resolution_clock::now() - start > std::chrono::seconds(Runforseconds)) \
                                             break; \
                                        }
#define MilliSleep(RunSeconds)          start = std::chrono::high_resolution_clock::now(); \
                                        Runforseconds = RunSeconds; \
                                        while(1) { \
                                        if (std::chrono::high_resolution_clock::now() - start > std::chrono::milliseconds(Runforseconds)) \
                                             break; \
                                        }
#define WaitForOperation                Sleep(5)
#define PLAYBACK_RATE_TOLERANCE         0.03
#define ENV_FILE                        "/opt/TDK/TDK.env"
#define DEBUG_PRINT(f_, ...)            if (enable_trace) \
                                            printf((f_), ##__VA_ARGS__)

char m_play_url[BUFFER_SIZE_LONG] = {'\0'};
char TDK_PATH[BUFFER_SIZE_SHORT] = {'\0'};
vector<string> operationsList;

/*
 * Default values for avstatus check flag and play_timeout if not received as input arguments
 */

bool enable_trace = false;
bool checkAVStatus = false;
int play_timeout = 10;
gint flags;
int Runforseconds;
auto start = std::chrono::high_resolution_clock::now();
bool latency_check_test = false;
auto timestamp = std::chrono::high_resolution_clock::now(), time_elapsed = std::chrono::high_resolution_clock::now();
auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(time_elapsed - timestamp);
bool firstFrameReceived = false;
bool checkPTS = true;
gint64 currentposition;
bool trickplay = false;
bool pause_operation = false;
bool forward_events = true;
gint64 startPosition;
string audiosink;
bool ignorePlayJump = false;
bool buffering_flag = true;
bool use_audioSink = true;
bool checkEachSecondPlayback = false;
bool checkEachSecondPTS = false;
bool justPrintPTS = false;
bool use_westerossink_fps = true;
bool checkNewPlay = false;
bool only_audio = false;
bool only_video = false;

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
 * Trickplay operations
 */
typedef enum {
  REWIND4x_RATE        = -4,
  REWIND3x_RATE         = -3,
  REWIND2x_RATE         = -2,
  FASTFORWARD2x_RATE    = 2,
  FASTFORWARD4x_RATE    = 4,
  FASTFORWARD3x_RATE   = 3
} PlaybackRates;

/*
 * Structure to pass arguments to/from the message handling method
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

typedef struct CustomData {
    GstElement *playbin;                /* Playbin element handle */
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
static void trickplayOperation (MessageHandlerData *data);
static gdouble getRate (GstElement* playbin);
static void SetupStream (MessageHandlerData *data);

void assert_failure(GstElement* playbin, bool success, const char *str= "Failure occured")
{
   if(success)
      return;
   if(playbin)
   {
      gst_element_set_state (playbin, GST_STATE_NULL);
   }
   gst_object_unref (playbin);
   fail_unless (false,str);
}

/*******************************************************************************************************************************************
Purpose:                To continue the state of the pipeline and check whether operation is being carried throughout the specified interval
Parameters:
playbin                   - The pipeline which is to be monitored
RunSeconds:               - The interval for which pipeline should be monitored
********************************************************************************************************************************************/
static void PlaySeconds(GstElement* playbin,int RunSeconds,bool seekOperation=false)
{
   gint64 currentPosition;
   gfloat _currentPosition;
   gfloat difference;
   GstMessage *message;
   GstBus *bus;
   MessageHandlerData data;
   gint64 pts;
   gint64 old_pts=0;
   gint pts_buffer=5;
   gint RanForTime=0;
   gdouble current_rate;
   GstElement *videoSink;
   GstState cur_state;
   gfloat play_jump = 0;
   gfloat play_jump_previous = 99;
   gfloat previous_position = 0;
   gint jump_buffer = 3;
   gint jump_buffer_small_value = 3;
   GstStateChangeReturn state_change;
   gfloat _startPosition = 0;


   /* Update data variables */
   data.playbin = playbin;
   data.setRateOperation = FALSE;
   data.terminate = FALSE;
   data.eosDetected = FALSE;

   g_object_get (playbin,"video-sink",&videoSink,NULL);

   gst_element_get_state (playbin, &cur_state, NULL, (GST_SECOND));

   if (only_audio)
   {
        checkPTS=false;
        use_westerossink_fps=false;
   }


   if (checkPTS)
   {	   
       g_object_get (videoSink,"video-pts",&pts,NULL);
       old_pts = pts;
   }

   printf("\nRunning for %d seconds, start Position is %lld\n",RunSeconds,startPosition/(GST_SECOND));
   assert_failure (playbin, gst_element_query_position (playbin, GST_FORMAT_TIME, &currentPosition), "Failed to query the current playback position");
   _currentPosition = currentPosition;
   previous_position = (_currentPosition/(GST_SECOND));

   if (pause_operation)
   {
        do
        {
            Sleep(1);
            printf("Current State is PAUSED , waiting for %d\n", RunSeconds);
            assert_failure (playbin, gst_element_query_position (playbin, GST_FORMAT_TIME, &currentPosition), "Failed to query the current playback position");
	    _currentPosition = currentPosition;
            play_jump = (_currentPosition/(GST_SECOND)) - previous_position;
            previous_position = (_currentPosition/(GST_SECOND));
	    printf("Current Position : %0.2f\n", (_currentPosition/GST_SECOND));

	    if (round(play_jump) != 0)
		jump_buffer -=1;

	    DEBUG_PRINT("\nin PAUSED state : jump_buffer : %d, play_jump : %f\n",jump_buffer,round(play_jump));
            assert_failure (playbin, jump_buffer != 0,"Playback is not PAUSED");
            if ((checkPTS) && (!seekOperation))
            {
                g_object_get (videoSink,"video-pts",&pts,NULL);
                printf("\nPTS: %lld \n",pts);
                if (old_pts != pts)
                {
                    pts_buffer -= 1;
                }
                assert_failure (playbin, pts_buffer != 0 , "Video is not PAUSED according to video-pts check of westerosSink");
                assert_failure (playbin, old_pts != 0 , "Video is not playing according to video-pts check of westerosSink");
                old_pts = pts;
            }
            RanForTime += 1;
        }while(RanForTime < RunSeconds);
        return;	   
   }

   if (trickplay)
   {
	jump_buffer = 3;
   }
   current_rate = getRate (playbin);
   printf("\nCurrent Playback rate is %f\n",current_rate);
   bus = gst_element_get_bus (playbin);
   do
   {
	Sleep(1);
        assert_failure (playbin, gst_element_query_position (playbin, GST_FORMAT_TIME, &currentPosition), "Failed to query the current playback position");
	_currentPosition = currentPosition;
	_startPosition = startPosition;
        difference = abs((_currentPosition/GST_SECOND) - (_startPosition/GST_SECOND));
        printf("\nCurrent Position : %0.2f , Playing after operation for: %0.2f",(_currentPosition/(GST_SECOND)),difference);


	DEBUG_PRINT("\nCurrent Position : %0.2f , Previous Position :%0.2f, jump_buffer : %d",(_currentPosition/(GST_SECOND)),previous_position,jump_buffer);
	play_jump = (_currentPosition/(GST_SECOND)) - previous_position;
	printf("\nPlay jump = %0.2f", play_jump);

	if(!trickplay)
	{  
	    play_jump = round(play_jump);

           /*
            * Ignore if first jump is 0
            */
            if (((int)play_jump != (int)NORMAL_PLAYBACK_RATE) && !(((play_jump == 0) && (play_jump_previous == 99))))
	    {
		DEBUG_PRINT("\njump_buffer is reduced at line number %d\n", __LINE__);
		DEBUG_PRINT("\tplay_jump %d, normal playback rate %d\n",(int)play_jump,(int)NORMAL_PLAYBACK_RATE);
                jump_buffer -=1;
            }
           /*
            * For small jumps until 2 , jump_buffer is 2
            */
            if ((jump_buffer == 0) && ((play_jump == 0) || (play_jump == 2) || (play_jump == -1)))
            {
 		DEBUG_PRINT("\njump_buffer is reduced at line number %d\n", __LINE__);
                jump_buffer_small_value -=1;
                jump_buffer = jump_buffer_small_value;
            }

           /*
            * if playbin reports jump=0 and then jump=2 , then video has played fine only
            */
            if (((play_jump == 2) && (play_jump_previous == 0)) && (jump_buffer == 0))
            {
		DEBUG_PRINT("\njump_buffer is reset at line number %d\n", __LINE__);
                jump_buffer = 1;
            }
        }
        else
	{
	    if (round(current_rate) == current_rate)
	    {
	        if (round(play_jump) != current_rate)
		{
                   DEBUG_PRINT("\njump_buffer is reduced at line number %d play_jump - current_rate %f\n", __LINE__,abs(play_jump - current_rate));			
		   jump_buffer -=1;
		}
	    }
	    else
	    {
		if ((abs(play_jump - current_rate) > PLAYBACK_RATE_TOLERANCE) && ((abs((play_jump_previous + play_jump)/2) - current_rate) > PLAYBACK_RATE_TOLERANCE))
		{
		   DEBUG_PRINT("\njump_buffer is reduced at line number %d\n", __LINE__);
		   jump_buffer -=1;
		}
	    }
        }

        message = gst_bus_pop_filtered (bus, (GstMessageType) ((GstMessageType) GST_MESSAGE_ERROR | (GstMessageType) GST_MESSAGE_EOS));
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
            }

            assert_failure (playbin, pts_buffer != 0 , "Video is not playing according to video-pts check of westerosSink");
            old_pts = pts;
        }
	
	if (!ignorePlayJump)
        {
            assert_failure (playbin, jump_buffer != 0 , "Playback is not happening at the expected rate");
        }

	previous_position = (_currentPosition/(GST_SECOND));
        play_jump_previous = play_jump;
   
   }while((difference <= RunSeconds) && !data.terminate && !data.eosDetected);

   if(data.eosDetected)
   {
	RanForTime = difference;
	printf("\nEnd of stream was detected, calling PlaySeconds agaifor remaining time %d\n",(RunSeconds-RanForTime));
	PlaySeconds(playbin,RunSeconds-RanForTime);
   }
   printf("\nExiting from PlaySeconds, currentPosition is %0.2f\n",_currentPosition/(GST_SECOND));
   gst_object_unref (bus);
}

void PlaybackValidation(MessageHandlerData *data, int seconds, bool seekOperation=false)
{
    data->westerosSink.frame_buffer = 3;
    data->audioSink.frame_buffer = 3;
    data->westerosSink.pts_buffer = 20;
    gdouble current_rate;
    gboolean video_frames_zero = false;
    gboolean video_pts_zero = false;

    if (only_audio)
    {
         checkPTS=false;
         use_westerossink_fps=false;
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
    // seconds paramters for loop monitoring
    int milliSeconds = 0;
    int previous_seconds = 0;
    int second_count = 0;
    // loop timer
    auto loopStart = std::chrono::high_resolution_clock::now();

    // Get pipeline state
    assert_failure (data->playbin, gst_element_get_state (data->playbin, &(data->cur_state), NULL, 0) == GST_STATE_CHANGE_SUCCESS);
    if (pause_operation)
	data->cur_state = GST_STATE_PAUSED;

    // Get playback rate
    current_rate = getRate (data->playbin);

    // For seek operation , start expected position from seekSeconds
    if (seekOperation)
    {
	printf ("\nValidating for seek operation seekSeconds = %lld\n",data->seekPosition);
	data->previousposition = data->seekPosition*GST_SECOND;
	// For seek followed by pause, rendered frames will be observed as 0
	//  which is expected as pipeline is flushed during seek and since its paused
	//  there will be no frames rendered on screen
	if (pause_operation)
	{
	    video_frames_zero = true;
	    video_pts_zero = true;
	}
    }

    while(1)
    {
	 /* Loop break condition   
	  * Break after executing the desired number of seconds
	  */
         if (std::chrono::high_resolution_clock::now() - loopStart > std::chrono::seconds(seconds))
              break;
     
	 // Second Counter
	 second_count = milliSeconds/1000;
	 /* Playback Position Validation
	  * By default Playback Position Validation is for each 100 milliseconds
	  * If user wants to override and make it for each second once checkEachSecondPlayback must be set to true
	  */
	 if (((checkEachSecondPlayback) && (second_count > previous_seconds)) || (!checkEachSecondPlayback))
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
                  fail_unless( abs(position_diff_seconds) < 0.9,"Difference in expected position is large \nTDK LOG :  %02d:%02d:%02d  -- Playback position: %" GST_TIME_FORMAT " Expected Playback position:  %" GST_TIME_FORMAT "    Position diff: %.5f seconds",timeStruct->tm_hour, timeStruct->tm_min, timeStruct->tm_sec,GST_TIME_ARGS(data->currentPosition),GST_TIME_ARGS(data->previousposition),abs(position_diff_seconds));
              }
	      if (data->cur_state == GST_STATE_PLAYING)
              {
                  data->previousposition += (100 * GST_MSECOND) * current_rate;
              }
	 }
         /* Video PTS validation - obtained from westerossink
          * Done for 100 milliseconds
          * If needed for each second validation instead of 100 milliseconds, use the below format in if condition
          * if ((checkPTS) && (second_count > previous_seconds))
          */
         if ((((checkEachSecondPTS) && (second_count > previous_seconds)) || (!checkEachSecondPTS)) && (checkPTS))
         {
             g_object_get (data->westerosSink.sink,"video-pts",&(data->westerosSink.pts),NULL);
	     if (!video_pts_zero)
                 printf("  PTS : %lld ",data->westerosSink.pts);
             if (((data->westerosSink.pts ==0) || (data->westerosSink.old_pts >= data->westerosSink.pts)) &&  (data->cur_state == GST_STATE_PLAYING))
             {
                 data->westerosSink.pts_buffer -= 1;
                 printf("\nWARNING : Video not playing");
             }
	     if ((((data->westerosSink.pts ==0) || (data->westerosSink.old_pts != data->westerosSink.pts)) &&  (data->cur_state == GST_STATE_PAUSED)) && (!video_pts_zero))
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
		 if (!video_frames_zero)
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
                 if ((data->audioSink.rendered_frames <= data->audioSink.previous_rendered_frames) &&  (data->cur_state == GST_STATE_PLAYING))
                    data->audioSink.frame_buffer -= 1;
		 else if ((data->audioSink.rendered_frames != data->audioSink.previous_rendered_frames) &&  (data->cur_state == GST_STATE_PAUSED))
		    data->audioSink.frame_buffer -= 1;
                 assert_failure (data->playbin,data->audioSink.frame_buffer != 0 , "Audio frames are not rendered properly");
             }
	     data->audioSink.previous_rendered_frames =  data->audioSink.rendered_frames;
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

static void parselatency()
{
    int latency_int;
    printf("\nTime measured: %.3lld milliseconds.\n", latency.count());
    latency_int = latency.count();
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
}

/********************************************************************************************************************
Purpose:               Callback function to set a variable to true on receiving first frame
*********************************************************************************************************************/
static void firstFrameCallback(GstElement *sink, guint size, void *context, gpointer data)
{
   bool *gotFirstFrameSignal = (bool*)data;
   printf ("\nReceived first frame signal\n");

   gst_element_query_position (sink, GST_FORMAT_TIME, &currentposition);
   printf("\nCurrent Position %lld\n",currentposition/(GST_SECOND));
   /*
    * Set the Value to global variable once the first frame signal is received
    */
   *gotFirstFrameSignal = true;
}

/********************************************************************************************************************
Purpose:               To get the current playback rate of pipeline
Parameters:
playbin [IN]          - GstElement* 'playbin' whose playback rate should be queried
Return:               - gdouble value for the current playback rate
*********************************************************************************************************************/
static gdouble getRate (GstElement* playbin)
{
    GstQuery *query;
    gdouble currentRate = 0.0;
    /*
     * Retrieve the current playback speed of the pipeline using gst_element_query()
     */
    /*
     * Create a GstQuery to retrieve the segment
     */
    query = gst_query_new_segment (GST_FORMAT_DEFAULT);
    /*
     * Query the playbin element
     */
    assert_failure (playbin, gst_element_query (playbin, query), "Failed to query the current playback rate");
    /*
     * Parse the GstQuery structure to get the current playback rate
     */
    gst_query_parse_segment (query, &currentRate, NULL, NULL, NULL);
    /*
     * Unreference the query structure
     */
    gst_query_unref (query);
    /*
     * The returned playback rate should be validated
     */
    return currentRate;
}

/********************************************************************************************************************
Purpose:               To check whether seek is successfull
Parameters:
playbin [IN]          - MessageHandleData contains all data of the pipeline
Return:               - NIL
*********************************************************************************************************************/

static void checkTrickplay(MessageHandlerData *Param)
{
    GstMessage *message;
    GstBus *bus;
    MessageHandlerData data;
    int Seek_time_threshold = 5;
    data.playbin = Param->playbin;
    bus = gst_element_get_bus (data.playbin);
    /*
     * Set all the required variables before polling for the message
     */
    data.terminate = FALSE;
    data.seeked = FALSE;
    data.setRateOperation = FALSE;
    if (Param->setRateOperation)
    {
        data.setRateOperation = TRUE;
        data.setRate = Param->setRate;
    }
    data.currentRate = 0.0;    
    data.seekPosition = Param->seekPosition;
    data.currentPosition = GST_CLOCK_TIME_NONE;
    data.stateChanged = FALSE;
    data.eosDetected = FALSE;

    if(!data.setRateOperation)
    {
         start = std::chrono::high_resolution_clock::now();
         while(!data.terminate && !data.seeked)
         {
               //Check if seek had already happened
               assert_failure (data.playbin, gst_element_query_position (data.playbin, GST_FORMAT_TIME, &(data.currentPosition)),
                                                     "Failed to query the current playback position");
               //Added (GST_SECOND) buffer time between currentPosition and seekPosition
               if (abs( data.currentPosition - data.seekPosition) <= ((GST_SECOND)))
               {
                   data.seeked = TRUE;
                   time_elapsed = std::chrono::high_resolution_clock::now();
               }

	       if (std::chrono::high_resolution_clock::now() - start > std::chrono::seconds(Seek_time_threshold))
                   break;
         }
    }
    else
    {
	 start = std::chrono::high_resolution_clock::now();
         do
         {
               message = gst_bus_pop_filtered (bus,(GstMessageType) ((GstMessageType) GST_MESSAGE_STATE_CHANGED |
                                   (GstMessageType) GST_MESSAGE_ERROR | (GstMessageType) GST_MESSAGE_EOS |
                                   (GstMessageType) GST_MESSAGE_ASYNC_DONE ));
               if (message != NULL)
               {
		    handleMessage (&data, message);
               }
               if (std::chrono::high_resolution_clock::now() - start > std::chrono::seconds(RATE_SET_TIMEOUT))
                    break;
         }while(!data.terminate && !data.seeked);
    }

    if(data.eosDetected == TRUE)
    {
	printf("\nEOS was detected, pipeline was reset, checking for trickplay change again\n");
	trickplayOperation(Param);
    }
    Param->seeked = data.seeked;
    Param->terminate = data.terminate;
    Param->currentRate = data.currentRate;
    Param->currentPosition = data.currentPosition;
    gst_object_unref (bus);
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
	    gst_element_set_state(data->playbin, GST_STATE_NULL);
            gst_object_unref (data->playbin);
            SetupStream (data);
            break;
        case GST_MESSAGE_STATE_CHANGED:
            data->stateChanged = TRUE;
        case GST_MESSAGE_ASYNC_DONE:
	    if (data->setRateOperation == TRUE)
            {
	       data->currentRate = getRate(data->playbin);	    
               if (data->setRate == data->currentRate)
               {
                   time_elapsed = std::chrono::high_resolution_clock::now();
                   data->seeked = TRUE;
               }
            }
            else
            {
               assert_failure (data->playbin, gst_element_query_position (data->playbin, GST_FORMAT_TIME, &(data->currentPosition)),
                                                     "Failed to querry the current playback position");
               //Added (GST_SECOND) buffer time between currentPosition and seekPosition
               if (abs( data->currentPosition - data->seekPosition) <= ((GST_SECOND)))
               {
                   data->seeked = TRUE;
                   time_elapsed = std::chrono::high_resolution_clock::now();
               }
            }
            break;
        case GST_MESSAGE_STREAM_START:
            data->streamStart = TRUE;
            break;
        default:
            break;
    }
    gst_message_unref (message);
}

/********************************************************************************************************************
Purpose:               To set the playback rate or to seek the positionof pieline
Parameters:
playbin [IN]          - MessageHandlerData element containing seekPosition or playback rate and playbin pipeline
Return:               - None
*********************************************************************************************************************/
static void trickplayOperation(MessageHandlerData *data)
{
    /*
     * Get the current playback position
     */
    assert_failure (data->playbin, gst_element_query_position (data->playbin, GST_FORMAT_TIME, &data->currentPosition), "Failed to query the current playback position");
    data->seeked = FALSE;
    if(!(data->setRateOperation))
    {
	data->seekPosition = (GST_SECOND) * (data->seekSeconds);
	timestamp = std::chrono::high_resolution_clock::now();
	assert_failure (data->playbin, gst_element_seek (data->playbin, NORMAL_PLAYBACK_RATE, GST_FORMAT_TIME,
                                   GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, data->seekPosition,
                                   GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE), "Failed to seek");
    }
    else
    {
	trickplay = true;
        GST_LOG ("Setting the playback rate to %f\n", data->setRate);
        /*
         * Playback rates can be positive or negative depending on whether we are fast forwarding or rewinding
         * Below are the Playback rates used in our test scenarios
         * Fastforward Rates:
         * 	1) 2.0
         * 	2) 3.0
         * 	3) 4.0
         * Rewind Rates:
         * 	1) -2.0
         * 	2) -3.0
         * 	3) -4.0
         */
        /*
         * Rewind the pipeline if rate is a negative number
         */ 
        timestamp = std::chrono::high_resolution_clock::now();
        if (data->setRate < 0)
	{
	     assert_failure (data->playbin, gst_element_seek (data->playbin, data->setRate, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                  GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, data->currentPosition), "Failed to set playback rate");
        }   
    	/*
         * Fast forward the pipeline if rate is a positive number
         */
        else
        {
             assert_failure (data->playbin, gst_element_seek(data->playbin, data->setRate, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, data->currentPosition,
    	        GST_SEEK_TYPE_SET, GST_CLOCK_TIME_NONE), "Failed to set playback rate");
        }    
    }

    checkTrickplay(data);

    if(!(data->setRateOperation))
    {
	//Convert time to seconds
        data->currentPosition /= (GST_SECOND);
        data->seekPosition /= (GST_SECOND);
	printf("\nCurrentPosition %lld seconds, SeekPosition %lld seconds\n", data->currentPosition, data->seekPosition);
        assert_failure (data->playbin, TRUE == data->seeked, "Seek Unsuccessfull\n");
        printf("\nSEEK SUCCESSFULL :  CurrentPosition %lld seconds, SeekPosition %lld seconds\n", data->currentPosition, data->seekPosition);
        GST_LOG ("\nSEEK SUCCESSFULL :  CurrentPosition %lld seconds, SeekPosition %lld seconds\n", data->currentPosition, data->seekPosition);
    }
    else
    {
	char playback_rate_string[100];
	sprintf(playback_rate_string,"Failed to do set rate to %f correctly\nCurrent playback rate is: %f\n", data->setRate, data->currentRate);
        assert_failure (data->playbin, data->setRate == data->currentRate, playback_rate_string);
        printf ("\nRate is set to %s %0.2fx speed\n",(data->setRate > 0)?"fastforward":"rewind", abs(data->setRate));
	if (data->setRate < 0)
        {
	    printf("\nIn negative rate handling");
	    bool reached_start = false;
	    gint64 previous_position;

	    if (currentposition/(GST_SECOND) == 0)
		reached_start = true;

            int time_to_reach_start = (currentposition/(GST_SECOND))/abs(data->currentRate);
            start = std::chrono::high_resolution_clock::now();

	    printf("\nTime to reach start = %d",time_to_reach_start);
            while(!reached_start)
            {
	       previous_position = currentposition;
               if ((currentposition/(GST_SECOND)) == 0)
               {
                  reached_start = true;
               }
               if (std::chrono::high_resolution_clock::now() - start > std::chrono::seconds(time_to_reach_start))
                  break;
               assert_failure (data->playbin, gst_element_query_position (data->playbin, GST_FORMAT_TIME, &currentposition), "Failed to query the current playback position");
	       if (previous_position != currentposition)
		  printf("\nCurrentPosition %lld seconds",(currentposition/(GST_SECOND)));
	    }

	    if (reached_start)
	    {
	       printf("\nPipeline successfully rewinded to start\n");
	       gst_element_set_state(data->playbin, GST_STATE_NULL);
	       gst_object_unref (data->playbin);
	       SetupStream (data);
	    }
            assert_failure (data->playbin, true == reached_start, "Pipeline was not able to rewind to start");
        }
    }
}

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
            printf("\nEnabled NATIVE_AUDIO flag for playbin\n");
        }
        if (getenv ("TDK_NATIVE_VIDEO") != NULL)
        {
            flags |= GST_PLAY_FLAG_NATIVE_VIDEO;
            printf("\nEnabled NATIVE_VIDEO flag for playbin\n");
        }
}

/********************************************************************************************************************
Purpose:               Setup stream
*********************************************************************************************************************/
static void SetupStream (MessageHandlerData *data)
{
    GstElement *playsink;
    GstStateChangeReturn state_change;
    /*
     * Create the playbin element
     */
    data->playbin = gst_element_factory_make(PLAYBIN_ELEMENT, NULL);
    fail_unless (data->playbin != NULL, "Failed to create 'playbin' element");
    /*
     * Set the url received from argument as the 'uri' for playbin
     */
    assert_failure (data->playbin, m_play_url != NULL, "Playback url should not be NULL");
    
    g_object_set (data->playbin, "uri", m_play_url, NULL);
    /*
     * Update the current playbin flags to enable Video and Audio Playback
     */
    g_object_get (data->playbin, "flags", &flags, NULL);
    setflags();
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
    /*
     * Set the first frame received callback
     */
    g_signal_connect(data->westerosSink.sink, "first-video-frame-callback", G_CALLBACK(firstFrameCallback), &firstFrameReceived);
    /*
     * Set the firstFrameReceived variable as false before starting play
     */
    firstFrameReceived= false;
    data->pipelineInitiation = true;
    data->westerosSink.previous_rendered_frames = 0;
    data->audioSink.previous_rendered_frames = 0;
    /*
     * Set playbin to PLAYING
     */
    GST_FIXME( "Setting to Playing State\n");
    assert_failure (data->playbin, gst_element_set_state (data->playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE);
    GST_FIXME( "Set to Playing State\n");
    do{
         state_change = gst_element_get_state (data->playbin, &(data->cur_state), NULL, 10000000);
    } while (state_change == GST_STATE_CHANGE_ASYNC);
    printf ("\n\n\nPipeline set to : %s  state \n\n\n", gst_element_state_get_name(data->cur_state));
    WaitForOperation;
    assert_failure (data->playbin, data->cur_state == GST_STATE_PLAYING, "Pipeline is not set to playing state");
    /*
     * Check if the first frame received flag is set
     */
    assert_failure (data->playbin, (only_audio) ||  (firstFrameReceived == true), "Failed to receive first video frame signal");
    if (checkNewPlay)
        PlaybackValidation(data,5);
    else
	PlaySeconds(data->playbin,5);
    
}

GST_START_TEST (trickplayTest)
{
    string operationString;
    double operationTimeout = 10.0;
    int seekSeconds = 0;
    bool seekOperation = false;
    gdouble rate = 1;
    GstBus *bus;
    GstStateChangeReturn state_change;
    GstState cur_state;
    MessageHandlerData data;
    int timeout = 0;
    bool is_av_playing = false;
    vector<string>::iterator operationItr;
    char* operationBuffer = NULL;

    SetupStream(&data);
    data.setRate = NORMAL_PLAYBACK_RATE;
    bus = gst_element_get_bus(data.playbin);
    gst_bus_add_watch(bus, (GstBusFunc) handleMessage, NULL);

    /*
     * Iterate through the list of operations recieved as input arguments and execute each of them for the requesed operation and timeperiod
     */
    for (operationItr = operationsList.begin(); operationItr != operationsList.end(); ++operationItr)
    {
	/*
	 * Operating string will be in operation:operationTimeout format,
	 * so split the string to retrieve operation string and timeout values
	 */
        operationBuffer = strdup ((*operationItr).c_str());
        operationString = strtok (operationBuffer, ":");
	operationTimeout = atof (strtok (NULL, ":"));
	if (!operationString.empty())
	{
	    timeout = operationTimeout;
        }
        if ("fastforward2x" == operationString)
        {
	    /*
	     * Fastforward the pipeline to 2
	     */
            rate = FASTFORWARD2x_RATE;
        }
	else if ("fastforward4x" == operationString)
        {
	    /*
	     * Fastforward the pipeline to 4
	     */
	    rate = FASTFORWARD4x_RATE;
	}
	else if ("fastforward3x" == operationString)
	{
	    /*
	     * Fastforward the pipeline to 3
	     */
	    rate = FASTFORWARD3x_RATE;	
	}
	else if ("rewind2x" == operationString)
	{
	    /*
	     * Rewind the pipeline to -2
	     */
	    rate = REWIND2x_RATE;
	}
	else if ("rewind4x" == operationString)
        {
	    /*
	     * Rewind the pipeline to -4
	     */
	    rate = REWIND4x_RATE;
	}
	else if ("rewind3x" == operationString)
	{
	    /*
	     * Rewind the pipeline to -3
	     */
	    rate = REWIND3x_RATE;
	}
	else if ("slowMotion0.5x" == operationString)
        {
            /*
             * Set the pipeline to 0.5x playback rate
             */
            rate = 0.5;
        }
	else if ("slowMotion0.75x" == operationString)
        {
            /*
             * Set the pipeline to 0.75x playback rate
             */
            rate = 0.75;
        }
	else if ("rate" == operationString)
	{
	    /*
	     * Set any rate mentioned
	     */
	    rate = atof(strtok(NULL, ":"));
	}
	else if ("seek" == operationString)
        {
	    /*
	     * Acquire seek seconds and set playabck rate to 1
	     */
	    seekSeconds = atoi(strtok(NULL, ":"));
	    seekOperation = true;
	    rate = 1;
	    if (pause_operation)
		    rate = 0;
	}
	else if ( ("play" == operationString) || ("pause" == operationString) )
        {
	    /*
	     * Playback rate is set to 1 if operation is play/pause
	     */
            rate = 1;
        }
	else
	{
	    GST_ERROR ("Invalid operation\n");
	}	
        
	if (rate != 0)
		pause_operation = false;

	data.currentRate = getRate(data.playbin);
        assert_failure (data.playbin, gst_element_query_position (data.playbin, GST_FORMAT_TIME, &data.currentPosition), "Failed to query the current playback position");

	if ((rate != data.currentRate) && (!pause_operation))
	{
	    printf("\nRequested playback rate is %f\n",rate);
	    if (rate < 0)
            {
                assert_failure (data.playbin, gst_element_query_duration (data.playbin, GST_FORMAT_TIME, &data.duration), "Failed to query the duration");
		printf("\nEntering negative rate operation\n");

	        /*Seek to  end of stream - 20 seconds for rewind testcases
		data.setRateOperation = FALSE;
	        data.seekSeconds = (abs(rate))*20;
		trickplayOperation(&data);*/
	    }
	    if ((rate > 0) && (rate < 1))
            {
                checkEachSecondPTS = true;
            }
	    data.setRateOperation = TRUE;
	    data.setRate = rate;
            trickplayOperation(&data);
	    if (!(rate < 0))
	    {
		if(!checkNewPlay)
		{
		    /* Playing for 20 seconds with 4x speed is equal to playing until position is 4*20 = 80 seconds */
                    operationTimeout *= abs(rate);
		}
            }
	    data.setRateOperation = FALSE;
	    WaitForOperation;
	    assert_failure (data.playbin, gst_element_query_position (data.playbin, GST_FORMAT_TIME, &startPosition), "Failed to query the current playback position");
	    if (latency_check_test)
            {
                latency = std::chrono::duration_cast<std::chrono::milliseconds>(time_elapsed - timestamp);
                parselatency();
            }

	}
        
    
	if (seekOperation)
	{
	    trickplay = false;
	    data.setRateOperation = FALSE;
	    data.seekSeconds = seekSeconds;
	    trickplayOperation(&data);
	    startPosition = seekSeconds * (GST_SECOND);
	    if (checkNewPlay)
            {
                if (!pause_operation)
                    data.seekPosition += 1;
		Sleep(1);
	    }
	    if (latency_check_test)
            {
                latency = std::chrono::duration_cast<std::chrono::milliseconds>(time_elapsed - timestamp);
                parselatency();
            }
	}

	if ("play" == operationString)
        {
	    trickplay = false;
	    /*
             * If pipeline is already in playing state with normal playback rate (1.0),
	     * just wait for operationTimeout seconds, instead os setting the pipeline to playing state again
	     */	
	    fail_unless_equals_int (gst_element_get_state (data.playbin, &cur_state,
                                                                  NULL, 0), GST_STATE_CHANGE_SUCCESS);
	    if ((cur_state != GST_STATE_PLAYING))
	    {
              	 /* Set the playbin state to GST_STATE_PLAYING
                  */
	         assert_failure (data.playbin, gst_element_set_state (data.playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE);
		 do{
		 //Waiting for state change
                    /*
                     * Polling for the state change to reflect with 10 ms timeout
                     */
                    state_change = gst_element_get_state (data.playbin, &cur_state, NULL, 10000000);
                 } while (state_change == GST_STATE_CHANGE_ASYNC);
		 printf ("\n********Current state is: %s \n", gst_element_state_get_name(cur_state));
             }
	     assert_failure (data.playbin, gst_element_query_position (data.playbin, GST_FORMAT_TIME, &startPosition), "Failed to query the current playback position");
             /*
	      * Wait for the requested time
	      */
             printf ("Waiting for %f seconds\n", operationTimeout);
	 } 
	 else if ("pause" == operationString)
         {
	     trickplay = false;
	     pause_operation = true;
             /*
	      * Set the playbin state to GST_STATE_PAUSED
	      */	
	     assert_failure (data.playbin, gst_element_query_position (data.playbin, GST_FORMAT_TIME, &startPosition), "Failed to query the current playback position");
	     gst_element_set_state (data.playbin, GST_STATE_PAUSED);
	     do{
                 //Waiting for state change
                 state_change = gst_element_get_state (data.playbin, &cur_state, NULL, 10000000);
             } while (state_change == GST_STATE_CHANGE_ASYNC);
	     assert_failure (data.playbin, gst_element_get_state (data.playbin, &cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS);
             assert_failure (data.playbin, cur_state == GST_STATE_PAUSED);
             printf("\n********Current state: %s\n",gst_element_state_get_name(cur_state));
             GST_LOG("\n********Current state: %s\n",gst_element_state_get_name(cur_state));
	     if (checkNewPlay)
	     {
		 Sleep(1);
		 checkEachSecondPlayback = true;
		 PlaybackValidation(&data,timeout);
		 checkEachSecondPlayback = false;
	     }
	     else
	     {
                 PlaySeconds(data.playbin,5);
	     }
	     /*
	      * Sleep for the requested time
	      */
	     operationTimeout -= 5;
	 }
	 if (true == checkAVStatus)
	 {    
	     is_av_playing = check_for_AV_status();
             assert_failure (data.playbin, is_av_playing == true, "Video is not playing in TV");
             printf ("DETAILS: SUCCESS, Video playing successfully \n");
 	 }

	 timeout=operationTimeout;
	 if (checkNewPlay)
	 {  
	     if (data.setRate != NORMAL_PLAYBACK_RATE)
	     {
		 printf( "\nSetRate Operation was invoked\n");
                 data.pipelineInitiation = true;
		 //Reset setRate
		 data.setRate = NORMAL_PLAYBACK_RATE;
	     }
             PlaybackValidation(&data,timeout,seekOperation);
	 }
	 else
         {
             PlaySeconds(data.playbin,timeout,seekOperation);
	 }
	 seekOperation = false;
    }
    if (data.playbin)
    {
       gst_element_set_state(data.playbin, GST_STATE_NULL);
    }
    /*
     * Cleanup after use
     */
    gst_object_unref (data.playbin);
    gst_object_unref(bus);
}
GST_END_TEST;

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
    int returnValue = 0;
    int arg = 0;
    char *operationStr = NULL;
    char *operation = NULL;
    double timeout = 0;
    Suite *gstPluginsSuite;
    TCase *tc_chain;

    /*
     * Get TDK path
     */
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
            printf ("Environment variables can be set in /opt/TDK/TDK.env\n");
            goto exit;
        }
    }

    if (getenv ("TDK_DEBUG") != NULL)
    {
        enable_trace = true;
    }

    if (argc < 2)
    {
        printf ("FALIURE : Insufficient arguments\n");
	goto exit;
    }

    strcpy(m_play_url,argv[1]);

    for (arg = 2; arg < argc; arg++)
    {
        if (strstr (argv[arg], "operations=") != NULL)
        {
            /*
             * The trickplay operations can be given in
             * operations="" argument as coma separated string
             * eg: operations=play:play_timeout,fastforward2x:timeout,seek:timeout:seekvalue
             */
            strtok (argv[arg], "=");
            operationStr = strtok(NULL, "=");
            operation = strtok (operationStr, ",");
            while (operation != NULL)
            {
               operationsList.push_back(operation);
               operation = strtok (NULL, ",");
            }
         }
         if (strcmp ("checkavstatus=yes", argv[arg]) == 0)
         {
            checkAVStatus = true;
	 }
	 if (strcmp ("checkPTS=no", argv[arg]) == 0)
         {
            checkPTS = false;
         }
	 if (strcmp ("checkLatency", argv[arg]) == 0)
	 {
	    latency_check_test = true;
	 }
	 if (strcmp ("ignorePlayJump", argv[arg]) == 0)
         {
            ignorePlayJump = true;
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
	 if (strcmp ("buffering_flag=no", argv[arg]) == 0)
         {
            buffering_flag = false;
         }
	 if (strcmp ("checkFPS=no", argv[arg]) == 0)
         {
            use_audioSink = false;
         }
	 if (strcmp ("checkAudioFPS=no", argv[arg]) == 0)
         {
            use_westerossink_fps = false;
         }
	 if (strcmp ("validateFullPlayback", argv[arg]) == 0)
         {
            checkNewPlay = true;
         }
	 if (strcmp ("checkEachSecondPlayback", argv[arg]) == 0)
         {
            checkEachSecondPlayback = true;
         }
	 if (strcmp ("checkEachSecondPTS", argv[arg]) == 0)
         {
            checkEachSecondPTS = true;
         }
	 if (strcmp ("only_audio", argv[arg]) == 0)
	 {
            only_audio = true;  		
	 }
    }
    gst_check_init (&argc, &argv);

    gstPluginsSuite = suite_create ("playbin_plugin_test");
    tc_chain = tcase_create ("general");
    /*
     * Set timeout to play_timeout if play_timeout > DEFAULT_TEST_SUITE_TIMEOUT(360) seconds
     */
    if (play_timeout > DEFAULT_TEST_SUITE_TIMEOUT)
    {
        timeout = play_timeout;
    }
    tcase_set_timeout (tc_chain, timeout);
    suite_add_tcase (gstPluginsSuite, tc_chain);
    tcase_add_test (tc_chain, trickplayTest);
    returnValue =  gst_check_run_suite (gstPluginsSuite, "playbin_plugin_test", __FILE__);
exit:
    return returnValue;
}
