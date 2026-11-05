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
#include <sys/stat.h>
#include <sstream>
#include <chrono>
#include <dlfcn.h>
#include <curl/curl.h>
#include <json/json.h>
#include <cstdio>
#include <fstream>

extern "C"
{
#include <gst/check/gstcheck.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include "GstTdkSrc.h"
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
#define DEBUG_PRINT(f_, ...)            if (enable_trace) \
                                            printf((f_), ##__VA_ARGS__)
#define LOG_FILE                        "/opt/TDK/mediapipeline_trickplay_test_step.log"
#define DOT_GENERATE_SETUP(playbin) if(playbin) generate_dot_graph(playbin, "setup")
#define DOT_GENERATE_PLAYING(playbin) if(playbin) generate_dot_graph(playbin, "playing")
#define DOT_GENERATE_FINAL(playbin) if(playbin) generate_dot_graph(playbin, "final")

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

/* Set to true by assert_failure() after it unrefs the pipeline.
 * The operations loop checks this flag at every iteration to avoid
 * dereferencing a freed data.playbin when GCheck CK_NOFORK mode
 * allows execution to continue after a failed fail_unless(). */
static bool g_pipeline_destroyed = false;
bool forward_events = true;
gint64 startPosition;
string audiosink;
bool ignorePlayJump = false;
std::string video_pts_validation_post_seek         = "FAILURE";   /* set in checkTrickplay */
std::string pipeline_position_validation_post_seek = "FAILURE";   /* set in checkTrickplay */
/* Rate-change scratch globals — overwritten per operation, then pushed to operation_verdicts */
std::string rate_op_type               = "";
double      rate_op_requested          = 0.0;
double      rate_op_confirmed          = 0.0;
std::string rate_confirmed_result      = "FAILURE";
std::string pos_advancing_result       = "FAILURE";
std::string pos_decreasing_result      = "FAILURE";
std::string reached_start_result       = "FAILURE";

/* Per-operation verdict — one entry pushed per trickplay/seek operation executed */
struct OperationVerdict {
    std::string op_type;          /* "seek", "fastforward", "rewind" */
    bool        overall_pass;
    /* seek fields */
    int         seek_target_sec;
    std::string pos_result;
    std::string pts_result;
    /* rate fields */
    double      rate_requested;
    double      rate_confirmed_val;
    std::string rate_confirmed_str;
    std::string pos_advancing_str;   /* fastforward */
    std::string pos_decreasing_str;  /* rewind */
    std::string reached_start_str;   /* rewind */
    OperationVerdict() : overall_pass(false), seek_target_sec(0),
                         rate_requested(0.0), rate_confirmed_val(0.0) {}
};
std::vector<OperationVerdict> operation_verdicts;
bool buffering_flag = true;
bool use_audioSink = true;
bool checkEachSecondPlayback = false;
bool checkEachSecondPTS = false;
bool justPrintPTS = false;
bool use_westerossink_fps = true;
bool checkNewPlay = true;
bool only_audio = false;
bool only_video = false;
bool westerosStarted = false;
bool curlError = false;
std::vector<void *> handles;
std::list<std::string> libHandles;
FILE *file = NULL;
bool log_enabled = true;
bool playbackValidationLog = false;
bool defaultStart = true;
bool startWesterosConfig = false;
bool createDisplayConfig = false;
bool displayCreated = false;
bool useWindowManager = false;
int returnValue = 0;

/* Multiappsrc support for container file playback */
bool use_multiappsrc = false;
bool bufferUnderflowTest = false;
bool multiappsrc_defer_eos = false;  /* For trickplay: defer EOS until after operations complete */
GstElement *multiappsrc_video_appsrc = NULL;  /* Reference to video appsrc for deferred EOS */
GstElement *multiappsrc_audio_appsrc = NULL;  /* Reference to audio appsrc for deferred EOS */
GQueue *multiappsrc_video_queue = NULL;  /* Reference to video buffer queue for pre-load */
GQueue *multiappsrc_audio_queue = NULL;  /* Reference to audio buffer queue for pre-load */
void *multiappsrc_video_app = NULL;  /* Opaque pointer to video App struct for updating iterator in pre-load */
void *multiappsrc_audio_app = NULL;  /* Opaque pointer to audio App struct for updating iterator in pre-load */
GList *multiappsrc_audio_buffer_iter = NULL;  /* Iterator for audio buffer queue (need-data position) */
guint64 total_bytes_fed = 0;
guint64 BYTES_THRESHOLD = 0;
guint64 trickplay_buffer_duration_ns = 0;  /* Dynamically calculated for trickplay operations */

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
  GstCaps *caps;                              /* Demuxed stream capabilities */
  guint64 demuxed_cumulative_duration_ns;     /* Track cumulative duration for demuxed queue feeding */
  gboolean segment_event_sent;                /* Flag to track if segment event has been sent */
  GList *current_buffer_iter;                 /* Iterator for current buffer during seek repositioning */
  guint64 current_byte_offset;                /* Byte offset tracker for seeking support */
};

typedef struct CustomData {
    GstElement *playbin;                /* Playbin element handle */
    App app;                            /* Appsrc data for multiappsrc mode */
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
static void tdkPreloadFastforward(App *videoApp, App *audioApp, gdouble trickplayRate);
void execute_postrequisite();

/********************************************************************************************************************
Purpose:               Generate DOT graph for GStreamer pipeline visualization
Parameters:
pipeline               - The GStreamer pipeline element
stage_name             - Stage identifier (e.g., "setup", "playing", "final")
********************************************************************************************************************/
void generate_dot_graph(GstElement *pipeline, const char *stage_name)
{
    if (!pipeline) {
        printf("[DOT] WARNING: Cannot generate dot graph - pipeline is NULL\n");
        return;
    }

    const char *dot_dir = g_getenv("GST_DEBUG_DUMP_DOT_DIR");
    if (!dot_dir) {
        printf("[DOT] GST_DEBUG_DUMP_DOT_DIR is not set, not proceeding to capture dot graph\n");
        return;
    }

    /* Append a timestamp so each capture gets a unique name and is not overwritten */
    char timestamp[32];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", tm_info);

    char filename[512];
    mkdir(dot_dir, 0755);
    snprintf(filename, sizeof(filename), "%s/MediaPipelineTests_Trickplay_%s_%s.dot", dot_dir, stage_name, timestamp);

    char just_filename[256];
    snprintf(just_filename, sizeof(just_filename), "MediaPipelineTests_Trickplay_%s_%s", stage_name, timestamp);

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


/********************************************************************************************************************
 * Purpose      : Print the verdict summary table and per-operation details for all trickplay
 *                operations recorded so far.  Called both from the post-teardown path and from
 *                assert_failure so the summary is always visible regardless of where the test stops.
 * Returns      : true if any operation failed, false if all passed (or no operations recorded).
 ********************************************************************************************************************/
static bool printOperationVerdicts()
{
    if (operation_verdicts.empty())
        return false;

    bool any_failed = false;

    /* ---- Summary table ---- */
    printf("\n");
    printf("================================================================\n");
    printf("          TRICKPLAY OPERATIONS - VERDICT SUMMARY               \n");
    printf("================================================================\n");
    for (size_t i = 0; i < operation_verdicts.size(); i++)
    {
        const OperationVerdict& v = operation_verdicts[i];
        if (v.op_type == "seek")
            printf("  Op %zu: %-12s (target=%ds)  -> %s\n",
                   i+1, "seek", v.seek_target_sec, v.overall_pass ? "PASS" : "FAIL");
        else if (v.op_type == "play" || v.op_type == "pause")
            printf("  Op %zu: %-12s               -> %s\n",
                   i+1, v.op_type.c_str(), v.overall_pass ? "PASS" : "FAIL");
        else
            printf("  Op %zu: %-12s (rate=%.2f)   -> %s\n",
                   i+1, v.op_type.c_str(), v.rate_requested, v.overall_pass ? "PASS" : "FAIL");
        if (!v.overall_pass) any_failed = true;
    }
    printf("  OVERALL RESULT : %s\n", any_failed ? "FAIL" : "PASS");
    printf("================================================================\n\n");

    if (log_enabled)
    {
        fprintf(file, "\n================================================================\n");
        fprintf(file, "          TRICKPLAY OPERATIONS - VERDICT SUMMARY               \n");
        fprintf(file, "================================================================\n");
        for (size_t i = 0; i < operation_verdicts.size(); i++)
        {
            const OperationVerdict& v = operation_verdicts[i];
            if (v.op_type == "seek")
                fprintf(file, "  Op %zu: %-12s (target=%ds)  -> %s\n",
                        i+1, "seek", v.seek_target_sec, v.overall_pass ? "PASS" : "FAIL");
            else if (v.op_type == "play" || v.op_type == "pause")
                fprintf(file, "  Op %zu: %-12s               -> %s\n",
                        i+1, v.op_type.c_str(), v.overall_pass ? "PASS" : "FAIL");
            else
                fprintf(file, "  Op %zu: %-12s (rate=%.2f)   -> %s\n",
                        i+1, v.op_type.c_str(), v.rate_requested, v.overall_pass ? "PASS" : "FAIL");
        }
        fprintf(file, "  OVERALL RESULT : %s\n", any_failed ? "FAIL" : "PASS");
        fprintf(file, "================================================================\n\n");
    }

    /* ---- Per-operation detailed verdict ---- */
    for (size_t i = 0; i < operation_verdicts.size(); i++)
    {
        const OperationVerdict& v = operation_verdicts[i];

        if (v.op_type == "seek")
        {
            bool pos_ok = (v.pos_result == "SUCCESS");
            bool pts_ok = (v.pts_result == "SUCCESS");

            printf("----------------------------------------------------------------\n");
            printf("  Op %zu: SEEK to %d s\n", i+1, v.seek_target_sec);
            printf("  pipeline_position_validation : %s\n", v.pos_result.c_str());
            printf("  video_pts_validation         : %s\n", v.pts_result.c_str());
            printf("  RESULT                       : %s\n", v.overall_pass ? "PASS" : "FAIL");
            printf("----------------------------------------------------------------\n");

            if (!pos_ok && pts_ok)
            {
                printf("[TESTCASE FAILURE REASON] pipeline_position_validation_post_seek = FAILURE\n");
                printf("  -> gst_element_query_position() did not report correct position after seek call.\n");
                printf("  -> Even though seek was validated via video-pts (video-pts = SUCCESS),\n");
                printf("     gst_element_query_position() returned an incorrect position.\n");
                printf("  -> The pipeline position clock may not have reset correctly after the flush-seek.\n");
            }
            else if (!pos_ok && !pts_ok)
            {
                printf("[TESTCASE FAILURE REASON] Seek unsuccessful.\n");
                printf("  -> Both gst_element_query_position() and video-pts (westerossink) failed\n");
                printf("     to reach the seek target position before the timeout expired.\n");
                printf("  -> The pipeline did not resume playback from the correct position after seek.\n");
            }
            else
            {
                printf("[RESULT] pipeline_position_validation = SUCCESS -> PASS.\n");
            }
            printf("\n");

            if (log_enabled)
            {
                fprintf(file, "----------------------------------------------------------------\n");
                fprintf(file, "  Op %zu: SEEK to %d s\n", i+1, v.seek_target_sec);
                fprintf(file, "  pipeline_position_validation : %s\n", v.pos_result.c_str());
                fprintf(file, "  video_pts_validation         : %s\n", v.pts_result.c_str());
                fprintf(file, "  RESULT                       : %s\n", v.overall_pass ? "PASS" : "FAIL");
                if (!pos_ok && pts_ok)
                    fprintf(file, "[TESTCASE FAILURE REASON] position wrong despite video-pts SUCCESS.\n");
                else if (!pos_ok && !pts_ok)
                    fprintf(file, "[TESTCASE FAILURE REASON] Seek unsuccessful — both validations FAILED.\n");
                else
                    fprintf(file, "[RESULT] PASS.\n");
                fprintf(file, "----------------------------------------------------------------\n\n");
            }
        }
        else if (v.op_type == "fastforward")
        {
            bool rate_ok = (v.rate_confirmed_str == "SUCCESS");
            bool pos_ok  = (v.pos_advancing_str  == "SUCCESS");

            printf("----------------------------------------------------------------\n");
            printf("  Op %zu: FASTFORWARD %.2fx\n", i+1, v.rate_requested);
            printf("  requested_rate     : %.2f\n", v.rate_requested);
            printf("  confirmed_rate     : %.2f\n", v.rate_confirmed_val);
            printf("  rate_confirmed     : %s\n",   v.rate_confirmed_str.c_str());
            printf("  position_advancing : %s\n",   v.pos_advancing_str.c_str());
            printf("  RESULT             : %s\n",   v.overall_pass ? "PASS" : "FAIL");
            printf("----------------------------------------------------------------\n");

            if (!rate_ok)
            {
                printf("[TESTCASE FAILURE REASON] Playback rate was not set correctly.\n");
                printf("  -> Requested %.2fx but pipeline reported %.2fx.\n",
                       v.rate_requested, v.rate_confirmed_val);
                printf("  -> Rate change did not take effect.\n");
            }
            else if (!pos_ok)
            {
                printf("[TESTCASE FAILURE REASON] Rate was set to %.2fx but "
                       "gst_element_query_position()\n", v.rate_requested);
                printf("  -> did not advance at the expected speed.\n");
                printf("  -> Pipeline may have accepted the rate change without "
                       "actually fast-forwarding.\n");
            }
            else
            {
                printf("[RESULT] Fastforward %.2fx SUCCESS — rate confirmed and "
                       "position advancing at expected rate.\n", v.rate_requested);
            }
            printf("\n");

            if (log_enabled)
            {
                fprintf(file, "----------------------------------------------------------------\n");
                fprintf(file, "  Op %zu: FASTFORWARD %.2fx\n", i+1, v.rate_requested);
                fprintf(file, "  rate_confirmed     : %s\n", v.rate_confirmed_str.c_str());
                fprintf(file, "  position_advancing : %s\n", v.pos_advancing_str.c_str());
                fprintf(file, "  RESULT             : %s\n", v.overall_pass ? "PASS" : "FAIL");
                if (!rate_ok)
                    fprintf(file, "[TESTCASE FAILURE REASON] Rate not set: requested %.2f got %.2f\n",
                            v.rate_requested, v.rate_confirmed_val);
                else if (!pos_ok)
                    fprintf(file, "[TESTCASE FAILURE REASON] Rate set but position not advancing.\n");
                else
                    fprintf(file, "[RESULT] Fastforward %.2fx SUCCESS.\n", v.rate_requested);
                fprintf(file, "----------------------------------------------------------------\n\n");
            }
        }
        else if (v.op_type == "rewind")
        {
            bool rate_ok    = (v.rate_confirmed_str  == "SUCCESS");
            bool pos_dec    = (v.pos_decreasing_str  == "SUCCESS");
            bool reached_ok = (v.reached_start_str   == "SUCCESS");

            printf("----------------------------------------------------------------\n");
            printf("  Op %zu: REWIND %.2fx\n", i+1, v.rate_requested);
            printf("  requested_rate      : %.2f\n", v.rate_requested);
            printf("  confirmed_rate      : %.2f\n", v.rate_confirmed_val);
            printf("  rate_confirmed      : %s\n",   v.rate_confirmed_str.c_str());
            printf("  position_decreasing : %s\n",   v.pos_decreasing_str.c_str());
            printf("  reached_start       : %s\n",   v.reached_start_str.c_str());
            printf("  RESULT              : %s\n",   v.overall_pass ? "PASS" : "FAIL");
            printf("----------------------------------------------------------------\n");

            if (!rate_ok)
            {
                printf("[TESTCASE FAILURE REASON] Rewind rate was not set correctly.\n");
                printf("  -> Requested %.2fx but pipeline reported %.2fx.\n",
                       v.rate_requested, v.rate_confirmed_val);
            }
            else if (!pos_dec && !reached_ok)
            {
                printf("[TESTCASE FAILURE REASON] Rewind rate %.2fx was accepted but "
                       "gst_element_query_position()\n", v.rate_requested);
                printf("  -> did not report a decreasing position.\n");
                printf("  -> Pipeline may be stalled after the rewind seek.\n");
            }
            else if (pos_dec && !reached_ok)
            {
                printf("[TESTCASE FAILURE REASON] Rewind %.2fx — position was decreasing "
                       "but pipeline\n", v.rate_requested);
                printf("  -> did not reach start within the expected time. Partial rewind only.\n");
            }
            else
            {
                printf("[RESULT] Rewind %.2fx SUCCESS — rate confirmed, position decreased, "
                       "and pipeline rewound to start.\n", v.rate_requested);
            }
            printf("\n");

            if (log_enabled)
            {
                fprintf(file, "----------------------------------------------------------------\n");
                fprintf(file, "  Op %zu: REWIND %.2fx\n", i+1, v.rate_requested);
                fprintf(file, "  rate_confirmed      : %s\n", v.rate_confirmed_str.c_str());
                fprintf(file, "  position_decreasing : %s\n", v.pos_decreasing_str.c_str());
                fprintf(file, "  reached_start       : %s\n", v.reached_start_str.c_str());
                fprintf(file, "  RESULT              : %s\n", v.overall_pass ? "PASS" : "FAIL");
                if (!rate_ok)
                    fprintf(file, "[TESTCASE FAILURE REASON] Rewind rate not set: "
                                  "requested %.2f got %.2f\n",
                                  v.rate_requested, v.rate_confirmed_val);
                else if (!pos_dec && !reached_ok)
                    fprintf(file, "[TESTCASE FAILURE REASON] Rate set but position not decreasing.\n");
                else if (pos_dec && !reached_ok)
                    fprintf(file, "[TESTCASE FAILURE REASON] Position decreasing but did not reach start.\n");
                else
                    fprintf(file, "[RESULT] Rewind %.2fx SUCCESS.\n", v.rate_requested);
                fprintf(file, "----------------------------------------------------------------\n\n");
            }
        }
        else if (v.op_type == "play" || v.op_type == "pause")
        {
            printf("----------------------------------------------------------------\n");
            printf("  Op %zu: %s\n", i+1, v.op_type == "play" ? "PLAY" : "PAUSE");
            printf("  RESULT : %s\n", v.overall_pass ? "PASS" : "FAIL");
            printf("[RESULT] %s operation completed successfully.\n",
                   v.op_type == "play" ? "Play" : "Pause");
            printf("----------------------------------------------------------------\n\n");

            if (log_enabled)
            {
                fprintf(file, "----------------------------------------------------------------\n");
                fprintf(file, "  Op %zu: %s\n", i+1, v.op_type == "play" ? "PLAY" : "PAUSE");
                fprintf(file, "  RESULT : %s\n", v.overall_pass ? "PASS" : "FAIL");
                fprintf(file, "[RESULT] %s operation completed successfully.\n",
                        v.op_type == "play" ? "Play" : "Pause");
                fprintf(file, "----------------------------------------------------------------\n\n");
            }
        }
    }

    return any_failed;
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
   /* Signal to the operations loop that the pipeline is gone.
    * Prevents use-after-free when GCheck CK_NOFORK lets execution
    * continue past the fail_unless() call below. */
   g_pipeline_destroyed = true;
   if (log_enabled)
   {
      fprintf(file,"\nFAILURE observed at %s : [%s %d]", func, __FILE__, line);
      fprintf(file,"\nFAILURE Reason : %s\n",str);
   }

   if (file)
   {
      printf("\nFCLOSE\n");
      fclose(file);
   }
   //execute_postrequisite();

   printOperationVerdicts();
   fail_unless(false,str);
}


void terminatePipeline(GstElement* playbin)
{
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

void PlaybackValidation(MessageHandlerData *data, int seconds, bool seekOperation=false, gint64 preSeekPositionNs=0)
{
    startPlaybackValidationLogging(true);

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
	assert_failure (data->playbin,gst_element_query_position (data->playbin, GST_FORMAT_TIME, &(data->previousposition)), "Failed to query the current playback position",
                        __FUNCTION__,__LINE__,"Querying Playback Position before starting Playback Validation" );
	if (log_enabled)
             fprintf(file, "\n");


	//assert_failure(data->playbin, GST_TIME_AS_SECONDS(data->previousposition) < 10, "Initial position of the pipeline is excessively high",
                        //__FUNCTION__,__LINE__,"Verify Pipeline initiates properly");

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
    // seconds paramters for loop monitoring
    int milliSeconds = 0;
    int previous_seconds = 0;
    int second_count = 0;
    int iterator =1;
    // loop timer
    auto loopStart = std::chrono::high_resolution_clock::now();


    if (log_enabled)
	 fprintf(file,"\n");

    // Get pipeline state
    assert_failure (data->playbin, gst_element_get_state (data->playbin, &(data->cur_state), NULL, 0) == GST_STATE_CHANGE_SUCCESS, "Failed to obtain playback state of pipeline",
                    __FUNCTION__,__LINE__,"Querying Pipeline State before starting Playback Validation");

    if (pause_operation)
    {
	if (log_enabled)
             fprintf(file, "\n");
	data->cur_state = GST_STATE_PAUSED;
    }

    // Get playback rate
    current_rate = getRate (data->playbin);

    // For seek operation , start expected position from pre-seek position
    // (after waitUntilPreSeekPositionReached the pipeline clock is expected to be
    //  at or near the pre-seek value, not the seek target)
    if (seekOperation)
    {
        if (log_enabled)
             fprintf(file, "\n");
	if (preSeekPositionNs > 0)
	{
	    printf ("\nValidating for seek operation: using pre-seek position %.3f s as baseline\n",
	            (double)preSeekPositionNs / GST_SECOND);
	    data->previousposition = preSeekPositionNs;
	}
	else
	{
	    printf ("\nValidating for seek operation seekSeconds = %lld\n",data->seekPosition);
	    data->previousposition = data->seekPosition*GST_SECOND;
	}
	// For seek followed by pause, rendered frames will be observed as 0
	//  which is expected as pipeline is flushed during seek and since its paused
	//  there will be no frames rendered on screen
	if (pause_operation)
	{
	    if (log_enabled)
                fprintf(file, "\n");
	    video_frames_zero = true;
	    video_pts_zero = true;
	}
    }

    while(1)
    {
	 if (log_enabled)
	      fprintf(file,"Iteration %02d ", iterator);
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

		 if (!video_frames_zero)
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


                 if ((data->audioSink.rendered_frames <= data->audioSink.previous_rendered_frames) &&  (data->cur_state == GST_STATE_PLAYING))
                    data->audioSink.frame_buffer -= 1;
                else if ((data->audioSink.rendered_frames > data->audioSink.previous_rendered_frames) &&  (data->cur_state == GST_STATE_PLAYING) && (data->audioSink.frame_buffer < 3))
                    data->audioSink.frame_buffer += 1;
		 else if ((data->audioSink.rendered_frames != data->audioSink.previous_rendered_frames) &&  (data->cur_state == GST_STATE_PAUSED))
		    data->audioSink.frame_buffer -= 1;

		 assert_failure (data->playbin,data->audioSink.frame_buffer != 0 , "Audio frames are not rendered properly",
                                 __FUNCTION__,__LINE__,"Verify Audio Frames rendered are incrementing");
             }
	     data->audioSink.previous_rendered_frames =  data->audioSink.rendered_frames;
         }
         // Seconds Counter
	 if (second_count > previous_seconds)
	 {
	     fprintf(file, "\n");
	     previous_seconds += 1;
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
    if (log_enabled)
        fprintf(file, "\nTime measured: %.3lld milliseconds.\n", latency.count());
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
    assert_failure (playbin, gst_element_query (playbin, query), "Failed to query the current playback rate",__FUNCTION__,__LINE__,"Queryig the current playback rate");
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
        /* Reset per-seek validation flags */
        video_pts_validation_post_seek         = "FAILURE";
        pipeline_position_validation_post_seek = "FAILURE";

        printf("\n[SEEK-CHECK] Polling position + video-pts (tolerance: 1 s, timeout: %d s)\n",
               Seek_time_threshold);

        /* Use a dedicated variable for the SEEK-CHECK timeout.
         * NOTE: MilliSleep() is a macro that overwrites the global `start` variable,
         * so we must NOT use `start` to measure the seek-check timeout window —
         * it would be reset on every sleep iteration, making the timeout never fire. */
        auto seek_check_start = std::chrono::high_resolution_clock::now();
        int seek_check_poll = 0;
        while(!data.terminate && !data.seeked)
        {
            /* Pipeline position check */
            assert_failure(data.playbin,
                           gst_element_query_position(data.playbin, GST_FORMAT_TIME, &(data.currentPosition)),
                           "Failed to query the current playback position",
                           __FUNCTION__, __LINE__, "Querying Playback Position");

            bool pos_ok = (abs(data.currentPosition - data.seekPosition) <= (gint64)GST_SECOND);

            /* Video-PTS check: pts within 1 s of seek target (90 kHz ticks) */
            gint64 vo_pts = 0;
            bool pts_ok = false;
            /* Skip video-pts validation for audio-only playback */
            if (only_audio)
            {
                pts_ok = true;  /* Mark as OK to skip video-pts validation */
            }
            else if (Param->westerosSink.sink)
            {
                g_object_get(Param->westerosSink.sink, "video-pts", &vo_pts, NULL);
                gint64 seek_ticks = (data.seekPosition / (gint64)GST_SECOND) * 90000LL;
                pts_ok = (llabs(vo_pts - seek_ticks) <= 90000LL);
            }

            {
                auto _ct = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::tm *_ts = std::localtime(&_ct);
                printf("\nTDK LOG :  %02d:%02d:%02d  --  video-pts : %lld  pts-progressing : %s  "
                       "playback-position : %.3f  position-progressing : %s",
                       _ts->tm_hour, _ts->tm_min, _ts->tm_sec,
                       (long long)vo_pts,
                       pts_ok ? "advancing" : "STALE",
                       (double)data.currentPosition / GST_SECOND,
                       pos_ok ? "advancing" : "STALE");
            }
            seek_check_poll++;

            if (pos_ok)
                pipeline_position_validation_post_seek = "SUCCESS";
            if (pts_ok)
                video_pts_validation_post_seek = "SUCCESS";

            /* Seek confirmed if at least one validation passes */
            if (pos_ok || pts_ok)
            {
                data.seeked = TRUE;
                time_elapsed = std::chrono::high_resolution_clock::now();
            }

            if (std::chrono::high_resolution_clock::now() - seek_check_start > std::chrono::seconds(Seek_time_threshold))
                break;

            MilliSleep(500);
        }

        printf("\n[SEEK-CHECK] pipeline_position_validation=%s  video_pts_validation=%s\n",
               pipeline_position_validation_post_seek.c_str(),
               video_pts_validation_post_seek.c_str());
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
    /* Generate DOT graph during trickplay operation */
    DOT_GENERATE_PLAYING(data->playbin);

    /*
     * Get the current playback position
     */
    
    assert_failure (data->playbin, gst_element_query_position (data->playbin, GST_FORMAT_TIME, &data->currentPosition), "Failed to query the current playback position", __FUNCTION__, __LINE__, "Query Playback Position");
    data->seeked = FALSE;
    if(!(data->setRateOperation))
    {
	data->seekPosition = (GST_SECOND) * (data->seekSeconds);
	timestamp = std::chrono::high_resolution_clock::now();

        /* Flush audio and video appsrc sources (MPB pattern) to prevent audio sync issues.
           Flush at the source rather than sink for better buffering control. */
        printf("[trickplay] Flushing appsrc sources before seek to position %.1f seconds\n", data->seekSeconds);
        
        /* Flush audio source if available */
        if (multiappsrc_audio_appsrc != NULL && !only_video)
        {
            printf("[trickplay] Flushing audio appsrc...\n");
            gst_element_send_event(multiappsrc_audio_appsrc, gst_event_new_flush_start());
            gst_element_send_event(multiappsrc_audio_appsrc, gst_event_new_flush_stop(FALSE));
        }
        
        /* Flush video source if available */
        if (multiappsrc_video_appsrc != NULL && !only_audio)
        {
            printf("[trickplay] Flushing video appsrc...\n");
            gst_element_send_event(multiappsrc_video_appsrc, gst_event_new_flush_start());
            gst_element_send_event(multiappsrc_video_appsrc, gst_event_new_flush_stop(FALSE));
        }
        
        g_usleep(10000);  /* 10ms for flush to propagate */
	
	gdouble seek_rate = (only_audio) ? 1.0 : NORMAL_PLAYBACK_RATE;
	
	assert_failure (data->playbin, gst_element_seek (data->playbin, seek_rate, GST_FORMAT_TIME,
                                   GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, data->seekPosition,
                                   GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE), "Failed to seek", __FUNCTION__, __LINE__, "Seek to the position");
	if (only_audio)
        {
	   if (log_enabled)
               fprintf(file, "\n");
           GstBus *bus;
           bus = gst_element_get_bus (data->playbin);
           GstMessage *message;
           start = std::chrono::high_resolution_clock::now();
           while (true)
           {
               message = gst_bus_pop_filtered (bus,(GstMessageType) ((GstMessageType) GST_MESSAGE_STATE_CHANGED |
                                     (GstMessageType) GST_MESSAGE_ERROR | (GstMessageType) GST_MESSAGE_EOS |
                                   (GstMessageType) GST_MESSAGE_ASYNC_DONE ));
               if ((message != NULL) && (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ASYNC_DONE))
               {
                    printf("\nBreaking due to Async message");
                    break;
               }
               if (std::chrono::high_resolution_clock::now() - start > std::chrono::seconds(2))
                   break;
           }

           gst_message_unref (message);
           gst_object_unref (bus);
        }
    }
    else
    {
	if (log_enabled)
            fprintf(file, "\n");
	trickplay = true;
        GST_LOG ("Setting the playback rate to %f\n", data->setRate);
        
        /* Flush audio and video appsrc sources (MPB pattern) before rate change.
           Clears accumulated frames so only new data flows during FF/RW. */
        printf("[trickplay] Flushing appsrc sources before rate change (rate=%.1fx)\n", data->setRate);
        
        /* Flush audio source if available */
        if (multiappsrc_audio_appsrc != NULL && !only_video)
        {
            printf("[trickplay] Flushing audio appsrc...\n");
            gst_element_send_event(multiappsrc_audio_appsrc, gst_event_new_flush_start());
            gst_element_send_event(multiappsrc_audio_appsrc, gst_event_new_flush_stop(FALSE));
        }
        
        /* Flush video source if available */
        if (multiappsrc_video_appsrc != NULL && !only_audio)
        {
            printf("[trickplay] Flushing video appsrc...\n");
            gst_element_send_event(multiappsrc_video_appsrc, gst_event_new_flush_start());
            gst_element_send_event(multiappsrc_video_appsrc, gst_event_new_flush_stop(FALSE));
        }
        
        g_usleep(10000);  /* 10ms for flush to propagate */
        
        /* Pre-load buffers ONLY for fastforward (not for seeks or rewind) 
           Fastforward = setRateOperation=TRUE && rate>1.0
           For 2x: 2 minutes buffer | For 3x/4x: 4 minutes buffer */
        
        gdouble trickplay_rate = (only_audio) ? 1.0 : data->setRate;
        
        printf("[trickplay] Seeking playbin with rate=%.1fx for %s\n", 
               trickplay_rate, only_audio ? "audio-only" : "video");
        
        /* For audio-only trickplay, rate=1.0 to playbin seek but audio decoder speed is set via server-play-speed */
        /* For video, use actual trickplay rate */
        
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
        
        /* Use FLUSH flag to trigger seek-data callback which repositions iterator.
           This is essential for both seek and rate-change operations to work correctly. */
        GstSeekFlags seek_flags = GST_SEEK_FLAG_FLUSH;
        
        printf("[trickplay] Current position before rate change seek: %" GST_TIME_FORMAT " (seconds: %.1f)\n",
               GST_TIME_ARGS(data->currentPosition), (double)data->currentPosition / GST_SECOND);
        
        /* Diagnostic logging for standalone fastforward debugging */
        if (use_multiappsrc && data->setRateOperation)
        {
            fprintf(stderr, "[trickplay-diagnostic] BEFORE rate change:\n");
            fprintf(stderr, "  currentPosition: %.3f seconds\n", (double)data->currentPosition / GST_SECOND);
            fprintf(stderr, "  video_queue size: %u buffers\n", 
                    multiappsrc_video_queue ? g_queue_get_length(multiappsrc_video_queue) : 0);
            fprintf(stderr, "  audio_queue size: %u buffers\n",
                    multiappsrc_audio_queue ? g_queue_get_length(multiappsrc_audio_queue) : 0);
            fprintf(stderr, "  video_iter: %p\n", 
                    multiappsrc_video_app ? ((App*)multiappsrc_video_app)->current_buffer_iter : NULL);
            fprintf(stderr, "  audio_iter: %p\n",
                    multiappsrc_audio_app ? ((App*)multiappsrc_audio_app)->current_buffer_iter : NULL);
        }
        
        if (data->setRate < 0)
	    {
	     assert_failure (data->playbin, gst_element_seek (data->playbin, trickplay_rate, GST_FORMAT_TIME, seek_flags,
                  GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, data->currentPosition), "Failed to set playback rate",__FUNCTION__,__LINE__,"Playback rate is set"); 
        }   
    	/*
         * Fast forward the pipeline if rate is a positive number
         */
        else
        {
             assert_failure (data->playbin, gst_element_seek(data->playbin, trickplay_rate, GST_FORMAT_TIME, seek_flags, GST_SEEK_TYPE_SET, data->currentPosition,
                GST_SEEK_TYPE_SET, GST_CLOCK_TIME_NONE), "Failed to set playback rate",__FUNCTION__,__LINE__,"Playback rate is set");
	}
        
        /* Manually reposition BOTH video and audio iterators to ensure they're at the correct position.
           This handles cases where seek-data callbacks don't fire for one or both streams.
           After FLUSH seek, tdkSeekData should have repositioned them, but we verify and fix if needed. */
        if (use_multiappsrc && multiappsrc_video_app != NULL && multiappsrc_audio_app != NULL)
        {
            App *video_app = (App *)multiappsrc_video_app;
            App *audio_app = (App *)multiappsrc_audio_app;
            
            printf("[trickplay] Manual iterator repositioning after seek to %.1f seconds\n", 
                   (double)data->currentPosition / GST_SECOND);
            
            /* Reposition video iterator */
            if (video_app->auQueue != NULL)
            {
                GList *iter = video_app->auQueue->head;
                GList *first_at_or_after = NULL;  /* First buffer at or after seek position */
                gboolean found = FALSE;
                
                while (iter != NULL)
                {
                    GstBuffer *buf = (GstBuffer *)iter->data;
                    guint64 buf_pts = GST_BUFFER_PTS(buf);
                    guint64 buf_duration = GST_BUFFER_DURATION(buf);
                    
                    if (buf_pts != GST_CLOCK_TIME_NONE && buf_duration != GST_CLOCK_TIME_NONE)
                    {
                        /* Check if seek position is within this buffer */
                        if (buf_pts <= data->currentPosition && data->currentPosition < buf_pts + buf_duration)
                        {
                            video_app->current_buffer_iter = iter;
                            printf("[trickplay] Video iterator repositioned to buffer at %" GST_TIME_FORMAT "\n", 
                                   GST_TIME_ARGS(buf_pts));
                            found = TRUE;
                            break;
                        }
                        
                        /* Track first buffer at or after seek position */
                        if (buf_pts >= data->currentPosition && first_at_or_after == NULL)
                            first_at_or_after = iter;
                    }
                    iter = iter->next;
                }
                
                if (!found)
                {
                    if (first_at_or_after != NULL)
                    {
                        video_app->current_buffer_iter = first_at_or_after;
                        GstBuffer *buf = (GstBuffer *)first_at_or_after->data;
                        printf("[trickplay] Video iterator not found within buffer, repositioned to first buffer at/after seek (%" GST_TIME_FORMAT ")\n", 
                               GST_TIME_ARGS(GST_BUFFER_PTS(buf)));
                    }
                    else if (video_app->auQueue->head != NULL)
                    {
                        video_app->current_buffer_iter = video_app->auQueue->head;
                        printf("[trickplay] Video iterator not found, repositioned to head\n");
                    }
                }
            }
            
            /* Reposition audio iterator */
            if (audio_app->auQueue != NULL)
            {
                GList *iter = audio_app->auQueue->head;
                GList *first_at_or_after = NULL;  /* First buffer at or after seek position */
                gboolean found = FALSE;
                
                while (iter != NULL)
                {
                    GstBuffer *buf = (GstBuffer *)iter->data;
                    guint64 buf_pts = GST_BUFFER_PTS(buf);
                    guint64 buf_duration = GST_BUFFER_DURATION(buf);
                    
                    if (buf_pts != GST_CLOCK_TIME_NONE && buf_duration != GST_CLOCK_TIME_NONE)
                    {
                        /* Check if seek position is within this buffer */
                        if (buf_pts <= data->currentPosition && data->currentPosition < buf_pts + buf_duration)
                        {
                            audio_app->current_buffer_iter = iter;
                            printf("[trickplay] Audio iterator repositioned to buffer at %" GST_TIME_FORMAT "\n", 
                                   GST_TIME_ARGS(buf_pts));
                            found = TRUE;
                            break;
                        }
                        
                        /* Track first buffer at or after seek position */
                        if (buf_pts >= data->currentPosition && first_at_or_after == NULL)
                            first_at_or_after = iter;
                    }
                    iter = iter->next;
                }
                
                if (!found)
                {
                    if (first_at_or_after != NULL)
                    {
                        audio_app->current_buffer_iter = first_at_or_after;
                        GstBuffer *buf = (GstBuffer *)first_at_or_after->data;
                        printf("[trickplay] Audio iterator not found within buffer, repositioned to first buffer at/after seek (%" GST_TIME_FORMAT ")\n", 
                               GST_TIME_ARGS(GST_BUFFER_PTS(buf)));
                    }
                    else if (audio_app->auQueue->head != NULL)
                    {
                        audio_app->current_buffer_iter = audio_app->auQueue->head;
                        printf("[trickplay] Audio iterator not found, repositioned to head\n");
                    }
                }
            }
        }
        
        printf("[trickplay] Rate change complete (rate=%.1fx), audio buffers flushed\n", data->setRate);
        
        /* Pre-load buffers AFTER seek completes (seek repositions iterator via tdkSeekData callback).
           Now current_buffer_iter is at the correct position for preload to start from.
           IMPORTANT: Pre-load BOTH video and audio in a single call to prevent confusion. */
        if (use_multiappsrc && data->setRateOperation && data->setRate > 1.0)
        {
            printf("[trickplay] Pre-loading buffers for fastforward at rate %.1fx\n", data->setRate);
            /* Defer EOS until after trickplay completes to allow pipeline to consume all pre-loaded buffers */
            multiappsrc_defer_eos = true;
            
            /* Pre-load both video and audio buffers in a single call */
            if (multiappsrc_video_app != NULL && !only_audio)
            {
                App *video_app = (App *)multiappsrc_video_app;
                App *audio_app = (multiappsrc_audio_app && !only_video) ? (App *)multiappsrc_audio_app : NULL;
                
                printf("[trickplay] Pre-loading video: iter=%p, audio: iter=%p\n",
                       video_app->current_buffer_iter, audio_app ? audio_app->current_buffer_iter : NULL);
                
                tdkPreloadFastforward(video_app, audio_app, data->setRate);
            }
        }
        

    }

    checkTrickplay(data);

    if(!(data->setRateOperation))
    {
	//Convert time to seconds
        data->currentPosition /= (GST_SECOND);
        data->seekPosition /= (GST_SECOND);
	printf("\nCurrentPosition %lld seconds, SeekPosition %lld seconds\n", data->currentPosition, data->seekPosition);
	if (log_enabled)
           fprintf(file, "\nCurrentPosition %lld seconds, SeekPosition %lld seconds\n", data->currentPosition, data->seekPosition);

        bool seek_ok = (pipeline_position_validation_post_seek == "SUCCESS");
        printf("\n[SEEK-RESULT] pipeline_position=%s  video_pts=%s  overall=%s\n",
               pipeline_position_validation_post_seek.c_str(),
               video_pts_validation_post_seek.c_str(),
               seek_ok ? "PASS" : "FAIL");

        /* Push per-operation verdict — final verdict is printed post-teardown */
        OperationVerdict _sv;
        _sv.op_type        = "seek";
        _sv.seek_target_sec = (int)data->seekPosition;
        _sv.pos_result     = pipeline_position_validation_post_seek;
        _sv.pts_result     = video_pts_validation_post_seek;
        _sv.overall_pass   = seek_ok;
        operation_verdicts.push_back(_sv);

        if (seek_ok)
        {
            printf("\nSEEK SUCCESSFULL :  CurrentPosition %lld seconds, SeekPosition %lld seconds\n", data->currentPosition, data->seekPosition);
            if (log_enabled)
               fprintf(file, "\nSEEK SUCCESSFULL :  CurrentPosition %lld seconds, SeekPosition %lld seconds\n", data->currentPosition, data->seekPosition);
            GST_LOG ("\nSEEK SUCCESSFULL :  CurrentPosition %lld seconds, SeekPosition %lld seconds\n", data->currentPosition, data->seekPosition);
        }
        else
        {
            printf("\n[SEEK-FAIL] Seek to %lld s FAILED — verdict will be printed after teardown.\n", data->seekPosition);
        }
    }
    else
    {
        bool rate_ok = (data->setRate == data->currentRate);

        /* Store rate info into globals for post-teardown verdict */
        rate_op_requested     = data->setRate;
        rate_op_confirmed     = data->currentRate;
        rate_confirmed_result = rate_ok ? "SUCCESS" : "FAILURE";

        if (data->setRate > 0)
        {
            rate_op_type = "fastforward";

            /* Sample position before/after 1 s to verify position is advancing */
            gint64 ff_pos_before = 0, ff_pos_after = 0;
            gst_element_query_position(data->playbin, GST_FORMAT_TIME, &ff_pos_before);
            Sleep(1);
            gst_element_query_position(data->playbin, GST_FORMAT_TIME, &ff_pos_after);
            bool pos_advancing = rate_ok && (ff_pos_after > ff_pos_before);
            pos_advancing_result = pos_advancing ? "SUCCESS" : "FAILURE";

            /* Push per-operation verdict */
            OperationVerdict _fv;
            _fv.op_type            = "fastforward";
            _fv.overall_pass       = rate_ok && pos_advancing;
            _fv.rate_requested     = data->setRate;
            _fv.rate_confirmed_val = data->currentRate;
            _fv.rate_confirmed_str = rate_confirmed_result;
            _fv.pos_advancing_str  = pos_advancing_result;
            operation_verdicts.push_back(_fv);

            printf("\nRate is set to fastforward %0.2fx speed\n", abs(data->setRate));
            
            /* Reset defer_eos flag now that trickplay has completed 
               This allows EOS to be emitted normally when buffers are exhausted */
            if (use_multiappsrc)
                multiappsrc_defer_eos = false;
        }
        else
        {
            rate_op_type = "rewind";

            printf("\nIn negative rate handling");
            bool reached_start = false;
            bool pos_decreasing = false;
            gint64 previous_position;

            if (currentposition/(GST_SECOND) == 0)
                reached_start = true;

            int time_to_reach_start = (currentposition/(GST_SECOND))/abs(data->currentRate);
            start = std::chrono::high_resolution_clock::now();

            printf("\nTime to reach start = %d", time_to_reach_start);
            while(!reached_start)
            {
                previous_position = currentposition;
                if ((currentposition/(GST_SECOND)) == 0)
                {
                    reached_start = true;
                }
                if (std::chrono::high_resolution_clock::now() - start > std::chrono::seconds(time_to_reach_start))
                    break;
                assert_failure(data->playbin,
                               gst_element_query_position(data->playbin, GST_FORMAT_TIME, &currentposition),
                               "Failed to query the current playback position",
                               __FUNCTION__, __LINE__, "Querying the current playback position");
                if (previous_position > currentposition)
                    pos_decreasing = true;
                if (previous_position != currentposition)
                    printf("\nCurrentPosition %lld seconds", (currentposition/(GST_SECOND)));
            }

            pos_decreasing_result = pos_decreasing ? "SUCCESS" : "FAILURE";
            reached_start_result  = reached_start  ? "SUCCESS" : "FAILURE";

            /* Push per-operation verdict */
            OperationVerdict _rv;
            _rv.op_type             = "rewind";
            _rv.overall_pass        = rate_ok && reached_start;
            _rv.rate_requested      = data->setRate;
            _rv.rate_confirmed_val  = data->currentRate;
            _rv.rate_confirmed_str  = rate_confirmed_result;
            _rv.pos_decreasing_str  = pos_decreasing_result;
            _rv.reached_start_str   = reached_start_result;
            operation_verdicts.push_back(_rv);

            if (reached_start)
            {
                printf("\nPipeline successfully rewinded to start\n");
                gst_element_set_state(data->playbin, GST_STATE_NULL);
                gst_object_unref(data->playbin);
                SetupStream(data);
            }
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
Purpose:               Multiappsrc Support Functions for Container File Demuxing
********************************************************************************************************************/

/* Pre-load buffers for fastforward: push buffers for 120s (2x) or 240s (3x/4x) of duration
 * Uses cumulative duration tracking (like MediaPipelineSuite_latest.cpp) to ensure exact duration
 * This ensures audio/video are available immediately without need-data starvation during FF
 * Only called for fastforward (not for seek or normal playback) */
static void tdkPreloadFastforward(App *videoApp, App *audioApp, gdouble trickplayRate)
{
    /* Determine buffer threshold based on trickplay rate - adds 30s safety margin like latest.cpp */
    guint64 base_preload_seconds = 2 * 60;  /* Default: 2 minutes for 2x */
    if (trickplayRate >= 3.0) {
        base_preload_seconds = 4 * 60;  /* 4 minutes for 3x, 4x rates */
    }
    guint64 safety_margin_seconds = 30;  /* Extra buffer to prevent starvation (same as latest.cpp) */
    guint64 target_preload_ns = (base_preload_seconds + safety_margin_seconds) * GST_SECOND;
    
    guint preload_count = 0;
    guint64 accumulated_video_duration_ns = 0;
    guint64 accumulated_audio_duration_ns = 0;
    GstFlowReturn ret;

    if (videoApp == NULL || videoApp->auQueue == NULL)
    {
        printf("[trickplay-preload] WARNING: videoApp or auQueue is NULL, skipping pre-load\n");
        return;
    }

    printf("\n[trickplay-preload] Pre-loading for rate=%.1fx: %.0f + %.0f sec margin = %.0f seconds total\n", 
           trickplayRate, (double)base_preload_seconds, (double)safety_margin_seconds, 
           (double)(base_preload_seconds + safety_margin_seconds));
    printf("[trickplay-preload] Video queue has %u buffers total\n", g_queue_get_length(videoApp->auQueue));
    
    /* After gst_element_seek() call above (with FLUSH flag), tdkSeekData callback is triggered.
       It repositions current_buffer_iter to the buffer at the current playback position (e.g., 10s).
       This ensures preload starts from the right position without re-pushing already-consumed buffers. */
    GList *iter = videoApp->current_buffer_iter;
    if (iter == NULL)
    {
        printf("[trickplay-preload] WARNING: current_buffer_iter is NULL after seek repositioning, cannot preload\n");
        return;
    }
    GList *last_pushed_iter = NULL;  /* Track the last buffer we successfully pushed */
    while (iter != NULL)
    {
        GstBuffer *buf = (GstBuffer *)iter->data;
        if (buf == NULL)
        {
            iter = iter->next;
            continue;
        }

        guint64 buf_duration = GST_BUFFER_DURATION(buf);
        guint64 buf_pts = GST_BUFFER_PTS(buf);
        
        /* Track cumulative duration (same logic as MediaPipelineSuite_latest.cpp) */
        if (buf_duration != GST_CLOCK_TIME_NONE)
            accumulated_video_duration_ns += buf_duration;

        /* Stop pre-loading once we've accumulated enough duration */
        if (accumulated_video_duration_ns >= target_preload_ns)
        {
            printf("[trickplay-preload] Video: reached target duration at PTS %" GST_TIME_FORMAT 
                   " (accumulated %" GST_TIME_FORMAT " >= target %" GST_TIME_FORMAT ")\n",
                   GST_TIME_ARGS(buf_pts), GST_TIME_ARGS(accumulated_video_duration_ns), 
                   GST_TIME_ARGS(target_preload_ns));
            break;
        }

        /* Push buffer to appsrc */
        if (videoApp->appsrc != NULL)
        {
            gst_buffer_ref(buf);  /* Extra ref for protection */
            ret = gst_app_src_push_buffer(GST_APP_SRC_CAST(videoApp->appsrc), buf);
            
            if (ret == GST_FLOW_OK)
            {
                preload_count++;
                last_pushed_iter = iter;  /* Track this as the last successfully pushed buffer */
                if (preload_count <= 5)  /* Only log first 5 to avoid spam */
                    printf("[trickplay-preload] Pushed buffer %u (PTS %" GST_TIME_FORMAT ")\n",
                           preload_count, GST_TIME_ARGS(buf_pts));
            }
            else
            {
                printf("[trickplay-preload] ERROR pushing buffer (ret=%s)\n", gst_flow_get_name(ret));
                break;
            }
        }
        else
        {
            printf("[trickplay-preload] WARNING: appsrc is NULL, cannot push buffers\n");
            break;
        }

        iter = iter->next;
    }
    
    printf("[trickplay-preload] COMPLETE: %u video buffers pre-loaded (%.2f seconds)\n", 
           preload_count, (double)accumulated_video_duration_ns / GST_SECOND);
    
    /* CRITICAL: Update video app's iterator and handle EOS for pre-load
       If we broke early (iter != NULL): resume from iter (first buffer after threshold)
       If we consumed all buffers (iter == NULL): emit EOS immediately to signal end-of-stream */
    if (iter != NULL)
    {
        videoApp->current_buffer_iter = iter;  /* Resume from first buffer after threshold */
        printf("[trickplay-preload] Video: resume from buffer after pre-load threshold\n");
    }
    else if (last_pushed_iter != NULL)
    {
        /* All buffers consumed */
        videoApp->current_buffer_iter = NULL;  /* Clear iterator */
        if (!multiappsrc_defer_eos)
        {
            /* Emit EOS immediately if not deferring */
            GstFlowReturn eos_ret = GST_FLOW_OK;
            g_signal_emit_by_name(videoApp->appsrc, "end-of-stream", &eos_ret);
            printf("[trickplay-preload] Video: all buffers pre-loaded, EOS emitted immediately\n");
        }
        else
        {
            /* Defer EOS until after trickplay completes */
            printf("[trickplay-preload] Video: all buffers pre-loaded, EOS deferred until trickplay completes\n");
        }
    }
    
    /* Pre-load audio buffers - use audioApp parameter if provided, otherwise use globals */
    App *active_audio_app = audioApp;  /* Use parameter if provided */
    if (active_audio_app == NULL && multiappsrc_audio_app != NULL)
        active_audio_app = (App *)multiappsrc_audio_app;  /* Fall back to global */
    
    if (active_audio_app != NULL && active_audio_app->auQueue != NULL && 
        active_audio_app->appsrc != NULL && !only_video)
    {
        guint audio_preload_count = 0;
        /* CRITICAL: Start from current audio position, not from queue->head (same fix as video).
           This prevents re-pushing already-consumed audio buffers. */
        GList *audio_iter = active_audio_app->current_buffer_iter;
        
        if (audio_iter == NULL)
        {
            printf("[trickplay-preload] WARNING: audio current_buffer_iter is NULL, cannot preload\n");
            return;
        }
        
        GList *last_pushed_iter = NULL;  /* Track the last buffer we successfully pushed */
        accumulated_audio_duration_ns = 0;
        
        printf("[trickplay-preload] Pre-loading audio buffers (target: %.0f seconds)\n",
               (double)target_preload_ns / GST_SECOND);
        
        while (audio_iter != NULL)
        {
            GstBuffer *audio_buf = (GstBuffer *)audio_iter->data;
            if (audio_buf == NULL)
            {
                audio_iter = audio_iter->next;
                continue;
            }
            
            guint64 audio_duration = GST_BUFFER_DURATION(audio_buf);
            guint64 audio_pts = GST_BUFFER_PTS(audio_buf);
            
            /* Track cumulative duration (same as video) */
            if (audio_duration != GST_CLOCK_TIME_NONE)
                accumulated_audio_duration_ns += audio_duration;
            
            /* Stop pre-loading once we've accumulated enough duration */
            if (accumulated_audio_duration_ns >= target_preload_ns)
            {
                printf("[trickplay-preload] Audio: reached target duration at PTS %" GST_TIME_FORMAT 
                       " (accumulated %" GST_TIME_FORMAT " >= target %" GST_TIME_FORMAT ")\n",
                       GST_TIME_ARGS(audio_pts), GST_TIME_ARGS(accumulated_audio_duration_ns),
                       GST_TIME_ARGS(target_preload_ns));
                break;
            }
            
            /* Push audio buffer to appsrc */
            gst_buffer_ref(audio_buf);
            ret = gst_app_src_push_buffer(GST_APP_SRC_CAST(active_audio_app->appsrc), audio_buf);
            
            if (ret == GST_FLOW_OK)
            {
                audio_preload_count++;
                last_pushed_iter = audio_iter;  /* Track this as the last successfully pushed buffer */
                if (audio_preload_count <= 5)  /* Only log first 5 to avoid spam */
                    printf("[trickplay-preload] Pushed audio buffer %u (PTS %" GST_TIME_FORMAT ")\n",
                           audio_preload_count, GST_TIME_ARGS(audio_pts));
            }
            else
            {
                printf("[trickplay-preload] ERROR pushing audio buffer (ret=%s)\n", gst_flow_get_name(ret));
                break;
            }
            
            audio_iter = audio_iter->next;
        }
        
        printf("[trickplay-preload] COMPLETE: %u audio buffers pre-loaded (%.2f seconds)\n", 
               audio_preload_count, (double)accumulated_audio_duration_ns / GST_SECOND);
        
        /* CRITICAL: Update the audio app's iterator and handle EOS for pre-load */
        if (audio_iter != NULL)
        {
            active_audio_app->current_buffer_iter = audio_iter;  /* Resume from first buffer after pre-load threshold */
            printf("[trickplay-preload] Audio: resume from buffer after pre-load threshold\n");
        }
        else if (last_pushed_iter != NULL)
        {
            /* All buffers consumed */
            active_audio_app->current_buffer_iter = NULL;  /* Clear iterator */
            if (!multiappsrc_defer_eos)
            {
                /* Emit EOS immediately if not deferring */
                GstFlowReturn eos_ret = GST_FLOW_OK;
                g_signal_emit_by_name(active_audio_app->appsrc, "end-of-stream", &eos_ret);
                printf("[trickplay-preload] Audio: all buffers pre-loaded, EOS emitted immediately\n");
            }
            else
            {
                /* Defer EOS until after trickplay completes */
                printf("[trickplay-preload] Audio: all buffers pre-loaded, EOS deferred until trickplay completes\n");
            }
        }
    }
}

/* Check if URL points to a container file (MP4, MOV, M4V, M4A, MKV, MKA) */
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

/* Structure for demux sink pair */
typedef struct _TdkDemuxSinks {
    GstElement *videoSink;
    GstElement *audioSink;
} TdkDemuxSinks;

/* Per-stream feed data for demuxing */
typedef struct _TdkFeedData {
    MessageHandlerData video;
    MessageHandlerData audio;
    GstCaps *videoCaps;
    GstCaps *audioCaps;
} TdkFeedData;

/* Callback when demux pads are added */
static void tdkDemuxPadAdded(GstElement *, GstPad *pad, gpointer user)
{
    TdkDemuxSinks *sinks = (TdkDemuxSinks *) user;
    GstCaps *caps = gst_pad_get_current_caps(pad);
    if (caps == NULL)
        caps = gst_pad_query_caps(pad, NULL);

    const gchar *name = gst_structure_get_name(gst_caps_get_structure(caps, 0));
    GstElement *sink = NULL;
    
    if (g_str_has_prefix(name, "video/"))
    {
        /* Skip video sink if only_audio is set */
        if (!only_audio)
            sink = sinks->videoSink;
        else
        {
            printf("\n[tdksrc] demux: skipping video pad (only_audio=true)\n");
            gst_caps_unref(caps);
            return;
        }
    }
    else if (g_str_has_prefix(name, "audio/"))
    {
        /* Skip audio sink if only_video is set */
        if (!only_video)
            sink = sinks->audioSink;
        else
        {
            printf("\n[tdksrc] demux: skipping audio pad (only_video=true)\n");
            gst_caps_unref(caps);
            return;
        }
    }

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

/* Drain appsink with optimized timeout for unused sinks */
static guint tdkDrainAppsink(GstElement *appsink, GQueue *queue, GstCaps **outCaps)
{
    guint count = 0;
    GstSample *sample;
    
    guint64 short_timeout_ns = 100 * GST_MSECOND;
    guint64 long_timeout_ns = 100 * GST_MSECOND;  /* Reduced from 5s to 100ms for faster drain */
    
    printf("[tdksrc] tdkDrainAppsink: starting drain with short timeout %llu ms\n", short_timeout_ns / GST_MSECOND);
    sample = gst_app_sink_try_pull_sample(GST_APP_SINK(appsink), short_timeout_ns);
    if (sample == NULL && *outCaps == NULL)
    {
        printf("[tdksrc] tdkDrainAppsink: no samples on first pull, returning 0\n");
        return 0;
    }
    
    if (sample != NULL)
    {
        printf("[tdksrc] tdkDrainAppsink: got first sample\n");
        if (*outCaps == NULL)
        {
            GstCaps *c = gst_sample_get_caps(sample);
            if (c != NULL)
            {
                *outCaps = gst_caps_copy(c);
                printf("[tdksrc] tdkDrainAppsink: copied caps from first sample\n");
            }
        }

        GstBuffer *buf = gst_sample_get_buffer(sample);
        if (buf != NULL)
        {
            g_queue_push_tail(queue, gst_buffer_ref(buf));
            count++;
            printf("[tdksrc] tdkDrainAppsink: queued first buffer (count=%u, size=%zu)\n", count, gst_buffer_get_size(buf));
        }
        gst_sample_unref(sample);
    }
    
    printf("[tdksrc] tdkDrainAppsink: entering fast drain loop with %llu ms timeout per pull\n", long_timeout_ns / GST_MSECOND);
    auto drain_start = std::chrono::high_resolution_clock::now();
    guint64 max_drain_time_ms = 3000;  /* Maximum 3 seconds total for drain */
    int loop_count = 0;
    
    while ((sample = gst_app_sink_try_pull_sample(GST_APP_SINK(appsink), long_timeout_ns)) != NULL)
    {
        loop_count++;
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
        
        /* Check if we've spent too long draining */
        auto elapsed = std::chrono::high_resolution_clock::now() - drain_start;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() > (long)max_drain_time_ms)
        {
            printf("[tdksrc] tdkDrainAppsink: drain timeout after %lld ms\n", 
                   std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
            break;
        }
    }
    printf("[tdksrc] tdkDrainAppsink: drain loop exited after %d iterations, %u total buffers\n", loop_count, count);
    
    gboolean is_eos = gst_app_sink_is_eos(GST_APP_SINK(appsink));
    printf("[tdksrc] tdkDrainAppsink: EOS=%d\n", is_eos);
    if (is_eos)
    {
        if (count > 0)
            printf("\n[tdksrc] tdkDrainAppsink: Stream reached EOS, extracted %u buffers\n", count);
        else
            printf("[tdksrc] tdkDrainAppsink: Stream reached EOS but 0 buffers extracted\n");
    }
    
    return count;
}

/* Calculate total size of all buffers in a queue for size property */
/* Need-data callback: feeds buffers dynamically to enable seeking
 * This is called when appsrc needs more data, allowing us to feed from
 * the current buffer iterator position (which can be changed by seek-data) */
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
            buffer = (GstBuffer *)app->current_buffer_iter->data;
            if (buffer != NULL)
            {
                guint64 buffer_duration = GST_BUFFER_DURATION(buffer);
                if (buffer_duration != GST_CLOCK_TIME_NONE)
                    app->demuxed_cumulative_duration_ns += buffer_duration;
                
                total_bytes_fed += gst_buffer_get_size(buffer);
                
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
            
            /* Move iterator to next buffer */
            app->current_buffer_iter = app->current_buffer_iter->next;
        }
        
        /* Check if queue is now exhausted (iterator became NULL) */
        if (app->current_buffer_iter == NULL && ret == GST_FLOW_OK)
        {
            /* All buffers have been pushed. Emit EOS unless deferred. */
            if (!multiappsrc_defer_eos)
            {
                g_signal_emit_by_name(app->appsrc, "end-of-stream", &ret);
            }
        }
        
        if (buffers_pushed_this_call > 0 && buffers_pushed_this_call <= 3)
        {
            /* Low push rate - may indicate queue starvation soon. Log for debugging. */
            /* printf("[tdksrc] WARNING: Low buffer feed rate (only %u buffers pushed)\n", buffers_pushed_this_call); */
        }
    }
    else
    {
        /* Iterator is NULL - either pre-load completed or all buffers consumed.
           Don't try to reposition, just emit EOS to signal end-of-stream. */
        if (!multiappsrc_defer_eos)
        {
            GstFlowReturn eos_ret = GST_FLOW_OK;
            g_signal_emit_by_name(appsrc, "end-of-stream", &eos_ret);
        }
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
    GstBuffer *last_buf = NULL;
    GList *last_iter = NULL;

    /* Get first and last buffer timestamps to detect backward seek beyond queue */
    if (iter != NULL)
    {
        first_buf = (GstBuffer *)iter->data;
        last_iter = g_queue_peek_tail_link(app->auQueue);
        if (last_iter != NULL)
            last_buf = (GstBuffer *)last_iter->data;
    }

    /* Check for BACKWARD seek to time BEFORE the queue starts */
    if (first_buf != NULL)
    {
        guint64 first_buf_pts = GST_BUFFER_PTS(first_buf);
        if (first_buf_pts != GST_CLOCK_TIME_NONE && offset < first_buf_pts)
        {
            /* Seeking backward before queue start (e.g., seek to 0 from position 10s) - reposition to HEAD */
            app->current_buffer_iter = app->auQueue->head;
            app->current_byte_offset = offset;
            printf("[tdksrc] seek-data: BACKWARD seek before queue, repositioned to HEAD (first buffer at %" GST_TIME_FORMAT ")\n",
                   GST_TIME_ARGS(first_buf_pts));
            return TRUE;
        }
    }

    /* Traverse queue to find buffer at or after requested time */
    while (iter != NULL)
    {
        GstBuffer *buf = (GstBuffer *)iter->data;
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

    /* If we get here, seek beyond current buffers (forward seek beyond end) - reposition to last buffer */
    if (last_iter != NULL)
    {
        app->current_buffer_iter = last_iter;
        printf("[tdksrc] seek-data: FORWARD seek beyond queue end, repositioned to TAIL (last buffer)\n");
        return TRUE;
    }

    app->current_byte_offset = offset;
    return TRUE;
}
} /* extern "C" */

/* Demux container file and extract buffers to queues */
static gboolean tdkDemuxContainer(const char *fileUrl, TdkFeedData *feed)
{
    const char *filePrefix = "file:";
    const char *filePath = fileUrl;
    if (strncmp(fileUrl, filePrefix, strlen(filePrefix)) == 0)
        filePath = fileUrl + strlen(filePrefix);

    /* For audio-only files (MP3, WAV, FLAC), use a direct audio pipeline without demuxing */
    if (tdkIsAudioOnly(fileUrl))
    {
        printf("\n[tdksrc] demux: audio-only file detected, using direct audio pipeline\n");
        
        GstElement *pipeline = gst_pipeline_new("tdk-audio-demux");
        GstElement *filesrc  = gst_element_factory_make("filesrc", NULL);
        GstElement *parser   = NULL;
        
        /* Use mpegaudioparse for MP3 to parse frames and extract raw encoded bytes */
        if (strstr(fileUrl, ".mp3") || strstr(fileUrl, ".MP3"))
        {
            parser = gst_element_factory_make("mpegaudioparse", NULL);
            if (parser)
                printf("[tdksrc] demux: using mpegaudioparse for MP3 frame parsing (raw encoded bytes)\n");
        }
        
        /* Fallback to decodebin for WAV, FLAC, AAC, OGG, etc. */
        if (!parser)
        {
            parser = gst_element_factory_make("decodebin", NULL);
            printf("[tdksrc] demux: using decodebin for audio format detection and decoding\n");
        }
        
        GstElement *asink = (!only_video) ? gst_element_factory_make("appsink", NULL) : NULL;

        if (!pipeline || !filesrc || !parser || (!only_video && !asink))
        {
            printf("\n[tdksrc] demux: failed to create audio extraction elements\n");
            if (pipeline) gst_object_unref(pipeline);
            if (parser) gst_object_unref(parser);
            if (asink) gst_object_unref(asink);
            return FALSE;
        }

        g_object_set(filesrc, "location", filePath, NULL);
        
        if (asink)
            g_object_set(asink, "sync", FALSE, "async", FALSE, "max-buffers", 0, "emit-signals", FALSE, NULL);

        if (asink)
            gst_bin_add_many(GST_BIN(pipeline), filesrc, parser, asink, NULL);
        else
            gst_bin_add_many(GST_BIN(pipeline), filesrc, parser, NULL);

        if (!gst_element_link(filesrc, parser))
        {
            printf("\n[tdksrc] demux: failed to link filesrc -> parser\n");
            gst_object_unref(pipeline);
            return FALSE;
        }

        if (!gst_element_link(parser, asink))
        {
            printf("\n[tdksrc] demux: failed to link parser -> appsink\n");
            gst_object_unref(pipeline);
            return FALSE;
        }

        printf("[tdksrc] demux: setting pipeline to PLAYING state...\n");
        GstState cur_state_audio;
        GstStateChangeReturn ret_state_audio = gst_element_set_state(pipeline, GST_STATE_PLAYING);
        
        if (ret_state_audio == GST_STATE_CHANGE_FAILURE)
        {
            printf("\n[tdksrc] demux: failed to start audio extraction pipeline\n");
            gst_object_unref(pipeline);
            return FALSE;
        }
        
        /* For ASYNC state changes, wait for completion (up to 3 seconds) */
        if (ret_state_audio == GST_STATE_CHANGE_ASYNC)
        {
            printf("[tdksrc] demux: audio pipeline state change is ASYNC, waiting for completion (max 3s)...\n");
            gint64 timeout_ns_audio = 3 * GST_SECOND;
            GstStateChangeReturn wait_ret_audio = gst_element_get_state(pipeline, &cur_state_audio, NULL, timeout_ns_audio);
            
            if (wait_ret_audio == GST_STATE_CHANGE_FAILURE)
            {
                printf("[ERROR] demux audio pipeline failed to reach PLAYING state\n");
                gst_object_unref(pipeline);
                return FALSE;
            }
            printf("[tdksrc] demux: audio pipeline reached state %s after async wait\n", gst_element_state_get_name(cur_state_audio));
        }
        else
        {
            printf("[tdksrc] demux: audio pipeline state change returned SUCCESS\n");
        }

        /* Additional wait for parser/decoder to process and produce buffers
           Broadcom: ~200-300ms, Realtek: ~800ms-1.5s for audio decode */
        printf("[tdksrc] demux: waiting 1.0s for audio parser/decoder to produce buffers...\n");
        g_usleep(1000000);  /* 1.0 second - covers both Broadcom and Realtek latencies */

        printf("[tdksrc] demux: draining appsink...\n");
        guint ac = asink ? tdkDrainAppsink(asink, feed->audio.app.auQueue, &feed->audioCaps) : 0;
        printf("[tdksrc] demux: extracted %u audio buffers from appsink\n", ac);

        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);

        printf("\n[tdksrc] demux: extracted 0 video AUs, %u audio AUs from %s (only_audio=%d, only_video=%d)\n",
                ac, filePath, only_audio, only_video);

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
    
    /* Only create sinks for streams we want to demux */
    GstElement *vsink    = (!only_audio) ? gst_element_factory_make("appsink", NULL) : NULL;
    GstElement *asink    = (!only_video) ? gst_element_factory_make("appsink", NULL) : NULL;

    if (!pipeline || !filesrc || !demux || 
        (!only_audio && !vsink) || (!only_video && !asink))
    {
        printf("\n[tdksrc] demux: failed to create extraction elements\n");
        if (pipeline) gst_object_unref(pipeline);
        if (vsink) gst_object_unref(vsink);
        if (asink) gst_object_unref(asink);
        return FALSE;
    }

    g_object_set(filesrc, "location", filePath, NULL);
    
    if (vsink)
        g_object_set(vsink, "sync", FALSE, "async", FALSE, "max-buffers", 0, "emit-signals", FALSE, NULL);
    if (asink)
        g_object_set(asink, "sync", FALSE, "async", FALSE, "max-buffers", 0, "emit-signals", FALSE, NULL);

    /* Use heap-allocated struct to avoid use-after-free when signal fires asynchronously */
    TdkDemuxSinks *sinks = (TdkDemuxSinks*)g_malloc(sizeof(TdkDemuxSinks));
    sinks->videoSink = vsink;
    sinks->audioSink = asink;
    
    /* Add only the elements we need */
    if (vsink && asink)
        gst_bin_add_many(GST_BIN(pipeline), filesrc, demux, vsink, asink, NULL);
    else if (vsink)
        gst_bin_add_many(GST_BIN(pipeline), filesrc, demux, vsink, NULL);
    else if (asink)
        gst_bin_add_many(GST_BIN(pipeline), filesrc, demux, asink, NULL);
    else
        gst_bin_add_many(GST_BIN(pipeline), filesrc, demux, NULL);
    
    if (!gst_element_link(filesrc, demux))
    {
        printf("\n[tdksrc] demux: failed to link filesrc -> demuxer (qtdemux/matroskademux)\n");
        g_free(sinks);
        gst_object_unref(pipeline);
        return FALSE;
    }
    gulong signal_id = g_signal_connect(demux, "pad-added", G_CALLBACK(tdkDemuxPadAdded), sinks);

    /* Set to PLAYING and wait for async state change to complete (critical for Realtek)
       Realtek: slower pipeline, requires waiting for filesrc pad activation and demux initialization
       Broadcom: faster pipeline, but proper wait ensures robustness */
    GstState cur_state;
    GstStateChangeReturn ret_state = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    
    if (ret_state == GST_STATE_CHANGE_FAILURE)
    {
        printf("\n[tdksrc] demux: failed to start extraction pipeline\n");
        g_signal_handler_disconnect(demux, signal_id);
        g_free(sinks);
        gst_object_unref(pipeline);
        return FALSE;
    }
    
    /* For ASYNC state changes, wait for completion (up to 3 seconds) */
    if (ret_state == GST_STATE_CHANGE_ASYNC)
    {
        printf("[tdksrc] demux: state change is ASYNC, waiting for completion (max 3s)...\n");
        gint64 timeout_ns = 3 * GST_SECOND;
        GstStateChangeReturn wait_ret = gst_element_get_state(pipeline, &cur_state, NULL, timeout_ns);
        
        if (wait_ret == GST_STATE_CHANGE_FAILURE)
        {
            printf("[ERROR] demux pipeline failed to reach PLAYING state\n");
            g_signal_handler_disconnect(demux, signal_id);
            g_free(sinks);
            gst_object_unref(pipeline);
            return FALSE;
        }
        printf("[tdksrc] demux: pipeline reached state %s after async wait\n", gst_element_state_get_name(cur_state));
    }
    else
    {
        printf("[tdksrc] demux: state change returned SUCCESS, pipeline ready\n");
    }

    /* Additional wait for demux pad-added callbacks to fire and link sinks
       Different platforms have different latencies:
       Broadcom: ~100-300ms, Realtek: ~500ms-1.5s due to hardware decode init
       This ensures filesrc pad is activated and demux has linked the pads */
    printf("[tdksrc] demux: waiting 1.0s for demux pads to link and buffers to start flowing...\n");
    g_usleep(1000000);  /* 1.0 second */

    guint vc = vsink ? tdkDrainAppsink(vsink, feed->video.app.auQueue, &feed->videoCaps) : 0;
    guint ac = asink ? tdkDrainAppsink(asink, feed->audio.app.auQueue, &feed->audioCaps) : 0;

    gst_element_set_state(pipeline, GST_STATE_NULL);
    g_signal_handler_disconnect(demux, signal_id);
    g_free(sinks);
    gst_object_unref(pipeline);

    printf("\n[tdksrc] demux: extracted %u video AUs, %u audio AUs from %s (only_audio=%d, only_video=%d)\n",
            vc, ac, filePath, only_audio, only_video);

    feed->video.app.demuxed = (vc > 0);
    feed->audio.app.demuxed = (ac > 0);
    return (vc > 0 || ac > 0);
}

/* Callback when tdksrc source element is set up */
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
        /* Set format to TIME for timestamp-based seeking (matches MPB approach) */
        gst_base_src_set_format(GST_BASE_SRC(feed->video.app.appsrc), GST_FORMAT_TIME);
        
        g_object_set(feed->video.app.appsrc, 
                     "caps", feed->videoCaps, 
                     "stream-type", GST_APP_STREAM_TYPE_SEEKABLE,
                     "format", GST_FORMAT_TIME,
                     "block", FALSE,
                     "is-live", FALSE,           /* File-based playback, not live stream */
                     "do-timestamp", FALSE,      /* Don't add timestamps, use PTS from buffers */
                     NULL);
        
        /* Redundant set for safety (matches MPB pattern) */
        gst_app_src_set_stream_type(GST_APP_SRC(feed->video.app.appsrc), GST_APP_STREAM_TYPE_SEEKABLE);
        
        /* Register seek-data callback for video appsrc */
        g_signal_connect(feed->video.app.appsrc, "seek-data",
                         G_CALLBACK(tdkSeekData), &feed->video.app);
        
        /* Register need-data callback for dynamic buffer feeding (enables seeking) */
        g_signal_connect(feed->video.app.appsrc, "need-data",
                         G_CALLBACK(tdkNeedData), &feed->video.app);
        feed->video.app.caps = gst_caps_copy(feed->videoCaps);
        feed->video.app.current_buffer_iter = feed->video.app.auQueue->head;
        feed->video.app.current_byte_offset = 0;
        printf("\n[tdksrc] source-setup: video demuxed caps set, stream-type=SEEKABLE, format=TIME, is-live=FALSE, seek-data callback registered\n");
    }

    if (feed->audio.app.demuxed && !only_video && feed->audio.app.appsrc != NULL)
    {
        /* Set format to TIME for timestamp-based seeking (matches MPB approach) */
        gst_base_src_set_format(GST_BASE_SRC(feed->audio.app.appsrc), GST_FORMAT_TIME);
        
        g_object_set(feed->audio.app.appsrc, 
                     "caps", feed->audioCaps, 
                     "stream-type", GST_APP_STREAM_TYPE_SEEKABLE,
                     "format", GST_FORMAT_TIME,
                     "block", FALSE,
                     "is-live", FALSE,           /* File-based playback, not live stream */
                     "do-timestamp", FALSE,      /* Don't add timestamps, use PTS from buffers */
                     NULL);
        
        /* Redundant set for safety (matches MPB pattern) */
        gst_app_src_set_stream_type(GST_APP_SRC(feed->audio.app.appsrc), GST_APP_STREAM_TYPE_SEEKABLE);
        
        /* Register seek-data callback for audio appsrc */
        g_signal_connect(feed->audio.app.appsrc, "seek-data",
                         G_CALLBACK(tdkSeekData), &feed->audio.app);
        
        /* Register need-data callback for dynamic buffer feeding (enables seeking) */
        g_signal_connect(feed->audio.app.appsrc, "need-data",
                         G_CALLBACK(tdkNeedData), &feed->audio.app);
        feed->audio.app.caps = gst_caps_copy(feed->audioCaps);
        feed->audio.app.current_buffer_iter = feed->audio.app.auQueue->head;
        feed->audio.app.current_byte_offset = 0;
        printf("\n[tdksrc] source-setup: audio demuxed caps set, stream-type=SEEKABLE, format=TIME, is-live=FALSE, seek-data callback registered\n");
    }

    printf("\n[tdksrc] source-setup: obtained tdksrc (video appsrc=%p, audio appsrc=%p)\n",
            (void *) feed->video.app.appsrc, (void *) feed->audio.app.appsrc);
}

/* Read demuxed buffers and push them into appsrc */
static gboolean read_data (MessageHandlerData * data)
{
  GstBuffer *buffer;
  GstFlowReturn ret;

  if (data == NULL || data->app.appsrc == NULL) {
    printf("\nReceived NULL pointer");
    return FALSE;
  }

    if (data->app.demuxed)
    {
        if (data->app.auQueue == NULL || g_queue_is_empty(data->app.auQueue))
        {
            /* For trickplay operations, defer EOS until after operations complete.
               For other modes, send EOS now to finish the stream. */
            if (!multiappsrc_defer_eos)
                g_signal_emit_by_name(data->app.appsrc, "end-of-stream", &ret);
            return FALSE;
        }

        /* Send sticky events in correct order: stream-start → caps → segment */
        if (!data->app.segment_event_sent && data->app.appsrc != NULL)
        {
            GstPad *srcpad = gst_element_get_static_pad(data->app.appsrc, "src");
            if (srcpad)
            {
                /* Send stream-start event first */
                GstEvent *stream_start_event = gst_event_new_stream_start("tdksrc-stream");
                if (!gst_pad_push_event(srcpad, stream_start_event))
                    printf("[tdksrc] Warning: Failed to push stream-start event\n");
                
                /* Send caps event second (required before segment) - MUST have caps */
                if (data->app.caps != NULL)
                {
                    GstEvent *caps_event = gst_event_new_caps(data->app.caps);
                    if (!gst_pad_push_event(srcpad, caps_event))
                        printf("[tdksrc] Warning: Failed to push caps event\n");
                }
                else
                {
                    printf("[tdksrc] Warning: No caps available for sticky event\n");
                }
                
                /* Send segment event after stream-start and caps */
                GstSegment segment;
                gst_segment_init(&segment, GST_FORMAT_TIME);
                segment.start = 0;
                segment.position = 0;
                GstEvent *segment_event = gst_event_new_segment(&segment);
                if (!gst_pad_push_event(srcpad, segment_event))
                    printf("[tdksrc] Warning: Failed to push segment event\n");
                
                gst_object_unref(srcpad);
                
                /* Small delay to allow sticky events to propagate downstream */
                g_usleep(10000);  /* 10ms */
            }
            data->app.segment_event_sent = TRUE;
        }

        /* For trickplay: push all buffers without duration limit.
           WaitForOperation macro will control when to stop. */
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

  return FALSE;
}

/* Calculate total buffer duration needed for all trickplay operations */
static guint64 calculateBufferDurationForTrickplay()
{
    guint64 total_duration_ns = 0;
    double operationTimeout = 0.0;
    int seekPosition = 0;
    char *operationBuffer = NULL;
    string operationString;
    
    printf("\n[tdksrc] Calculating buffer duration for trickplay operations...\n");
    
    for (auto &op : operationsList)
    {
        operationBuffer = strdup(op.c_str());
        operationString = strtok(operationBuffer, ":");
        
        if (operationString.empty())
        {
            free(operationBuffer);
            continue;
        }
        
        /* Parse timeout (first numeric parameter) */
        char *timeout_str = strtok(NULL, ":");
        operationTimeout = timeout_str ? atof(timeout_str) : 0.0;
        
        guint64 op_duration_ns = 0;
        
        if (operationString == "play")
        {
            /* play:T -> push T seconds */
            op_duration_ns = (guint64)(operationTimeout * GST_SECOND);
            printf("  [play:%.0f] -> %" G_GUINT64_FORMAT "ns\n", operationTimeout, op_duration_ns);
        }
        else if (operationString == "pause")
        {
            /* pause:T -> push T seconds (buffered, not consumed) */
            op_duration_ns = (guint64)(operationTimeout * GST_SECOND);
            printf("  [pause:%.0f] -> %" G_GUINT64_FORMAT "ns\n", operationTimeout, op_duration_ns);
        }
        else if (operationString == "seek")
        {
            /* seek:T:P -> push (T + P) seconds (T = play after seek, P = seek position) */
            char *seek_pos_str = strtok(NULL, ":");
            seekPosition = seek_pos_str ? atoi(seek_pos_str) : 0;
            op_duration_ns = (guint64)((operationTimeout + seekPosition) * GST_SECOND);
            printf("  [seek:%.0f:%d] -> (%.0f + %d) = %" G_GUINT64_FORMAT "ns\n", 
                   operationTimeout, seekPosition, operationTimeout, seekPosition, op_duration_ns);
        }
        else if (operationString == "fastforward2x")
        {
            /* fastforward2x:T -> push (2 * T) seconds (consumed 2x faster) */
            op_duration_ns = (guint64)(2.0 * operationTimeout * GST_SECOND);
            printf("  [fastforward2x:%.0f] -> (2 * %.0f) = %" G_GUINT64_FORMAT "ns\n", 
                   operationTimeout, operationTimeout, op_duration_ns);
        }
        else if (operationString == "fastforward3x")
        {
            /* fastforward3x:T -> push (3 * T) seconds (consumed 3x faster) */
            op_duration_ns = (guint64)(3.0 * operationTimeout * GST_SECOND);
            printf("  [fastforward3x:%.0f] -> (3 * %.0f) = %" G_GUINT64_FORMAT "ns\n", 
                   operationTimeout, operationTimeout, op_duration_ns);
        }
        else if (operationString == "rewind2x")
        {
            /* rewind2x:T -> push (2 * T) seconds (consumed 2x faster backward) */
            op_duration_ns = (guint64)(2.0 * operationTimeout * GST_SECOND);
            printf("  [rewind2x:%.0f] -> (2 * %.0f) = %" G_GUINT64_FORMAT "ns\n", 
                   operationTimeout, operationTimeout, op_duration_ns);
        }
        else if (operationString == "fastforward4x")
        {
            /* fastforward4x:T -> push (4 * T) seconds (consumed 4x faster) */
            op_duration_ns = (guint64)(4.0 * operationTimeout * GST_SECOND);
            printf("  [fastforward4x:%.0f] -> (4 * %.0f) = %" G_GUINT64_FORMAT "ns\n", 
                   operationTimeout, operationTimeout, op_duration_ns);
        }
        else if (operationString == "rewind3x")
        {
            /* rewind3x:T -> push (3 * T) seconds (consumed 3x faster backward) */
            op_duration_ns = (guint64)(3.0 * operationTimeout * GST_SECOND);
            printf("  [rewind3x:%.0f] -> (3 * %.0f) = %" G_GUINT64_FORMAT "ns\n", 
                   operationTimeout, operationTimeout, op_duration_ns);
        }
        else if (operationString == "rewind4x")
        {
            /* rewind4x:T -> push (4 * T) seconds (consumed 4x faster backward) */
            op_duration_ns = (guint64)(4.0 * operationTimeout * GST_SECOND);
            printf("  [rewind4x:%.0f] -> (4 * %.0f) = %" G_GUINT64_FORMAT "ns\n", 
                   operationTimeout, operationTimeout, op_duration_ns);
        }
        
        total_duration_ns += op_duration_ns;
        free(operationBuffer);
    }
    
    /* Add 20 seconds threshold */
    guint64 threshold_ns = 20 * GST_SECOND;
    total_duration_ns += threshold_ns;
    
    printf("\n[tdksrc] Total trickplay buffer duration: %" G_GUINT64_FORMAT "ns (~%u seconds) (including 20s threshold)\n",
           total_duration_ns, (guint)(total_duration_ns / GST_SECOND));
    
    return total_duration_ns;
}

/* Setup multiappsrc mode with demuxed container playback */
static gboolean setupInlineMultiAppsrc(MessageHandlerData *data, bool play_after_setup)
{
    GstState cur_state;
    gint64 timeout_ns;
    GstStateChangeReturn ret_state;  /* Declare before any goto to avoid cross-initialization */
    GstFlowReturn ret_feed = GST_FLOW_OK;
    guint video_preload_count = 0, audio_preload_count = 0;

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

    /* For trickplay operations (play_after_setup=true): defer EOS emission until after operations complete.
       Set this BEFORE setting state so it takes effect from the start. */
    if (play_after_setup)
    {
        multiappsrc_defer_eos = true;
        printf("[tdksrc] Deferring EOS for trickplay operations\n");
    }

    /* Set to PAUSED first (not PLAYING) to allow source-setup callback to complete
       and appsrc to be created without triggering preroll/EOS prematurely */
    if (gst_element_set_state(data->playbin, GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE)
        return FALSE;

    /* Brief delay for source-setup callback to complete */
    g_usleep(50000);  /* 50ms */

    if ((only_audio || feed.video.app.appsrc != NULL) && (only_video || feed.audio.app.appsrc != NULL))
    {
        printf("[tdksrc] Got valid appsrc from tdksrc\n");
        if (play_after_setup)
        {
            multiappsrc_video_appsrc = feed.video.app.appsrc;
            multiappsrc_audio_appsrc = feed.audio.app.appsrc;
            
            /* Copy queue and caps to data->app for trickplay pre-load access */
            data->app.auQueue = feed.video.app.auQueue;
            data->app.appsrc = feed.video.app.appsrc;
            data->app.caps = feed.videoCaps;
            data->app.current_buffer_iter = feed.video.app.current_buffer_iter;
            
            /* Store queue, appsrc, and App pointers for pre-load access */
            multiappsrc_video_queue = feed.video.app.auQueue;
            multiappsrc_video_appsrc = feed.video.app.appsrc;
            multiappsrc_video_app = (void *)&feed.video.app;  /* Store opaque pointer to video App struct */
            
            multiappsrc_audio_queue = feed.audio.app.auQueue;
            multiappsrc_audio_appsrc = feed.audio.app.appsrc;
            multiappsrc_audio_app = (void *)&feed.audio.app;  /* Store opaque pointer to audio App struct */
            multiappsrc_audio_buffer_iter = feed.audio.app.current_buffer_iter;
        }
    }
    else
    {
        printf("[ERROR] Failed to get appsrc from tdksrc\n");
        return FALSE;
    }

    printf("\nBuffer feeding via need-data callbacks (dynamic mode - supports seeking)\n");
    /* Buffers are now fed on-demand via tdkNeedData callback when pipeline needs them.
       This enables seeking: seek-data repositions the iterator, next need-data feeds from new position.
       
       For trickplay: Push buffers upfront to ensure initial preroll succeeds, 
       but keep queue intact for preloading during rate changes. */
    
    /* Push buffers upfront to enable PLAYING state to reach quickly */
    printf("[tdksrc] Pre-feeding buffers for initial playback...\n");
    
    /* Pre-feed video buffers (iterate without removing from queue) */
    if (!only_audio && feed.video.app.appsrc != NULL && feed.video.app.auQueue != NULL)
    {
        GList *iter = feed.video.app.auQueue->head;
        guint64 accumulated_duration = 0;
        guint64 preload_target_ns = 10 * GST_SECOND;  /* 10 seconds initial preload for robust PLAYING state transition */
        
        while (iter != NULL && accumulated_duration < preload_target_ns)
        {
            GstBuffer *buf = (GstBuffer *)iter->data;
            if (buf != NULL)
            {
                gst_buffer_ref(buf);
                ret_feed = gst_app_src_push_buffer(GST_APP_SRC_CAST(feed.video.app.appsrc), buf);
                if (ret_feed == GST_FLOW_OK)
                {
                    video_preload_count++;
                    guint64 buf_duration = GST_BUFFER_DURATION(buf);
                    if (buf_duration != GST_CLOCK_TIME_NONE)
                        accumulated_duration += buf_duration;
                }
                else
                {
                    printf("[tdksrc] Error pushing video buffer during pre-feed\n");
                    break;
                }
            }
            iter = iter->next;
        }
        printf("[tdksrc] Pre-fed %u video buffers (%.1f seconds)\n", 
               video_preload_count, (double)accumulated_duration / GST_SECOND);
    }
    
    /* Pre-feed audio buffers (iterate without removing from queue) */
    if (!only_video && feed.audio.app.appsrc != NULL && feed.audio.app.auQueue != NULL)
    {
        GList *iter = feed.audio.app.auQueue->head;
        guint64 accumulated_duration = 0;
        guint64 preload_target_ns = 10 * GST_SECOND;  /* 10 seconds initial preload for robust PLAYING state transition */
        
        while (iter != NULL && accumulated_duration < preload_target_ns)
        {
            GstBuffer *buf = (GstBuffer *)iter->data;
            if (buf != NULL)
            {
                gst_buffer_ref(buf);
                ret_feed = gst_app_src_push_buffer(GST_APP_SRC_CAST(feed.audio.app.appsrc), buf);
                if (ret_feed == GST_FLOW_OK)
                {
                    audio_preload_count++;
                    guint64 buf_duration = GST_BUFFER_DURATION(buf);
                    if (buf_duration != GST_CLOCK_TIME_NONE)
                        accumulated_duration += buf_duration;
                }
                else
                {
                    printf("[tdksrc] Error pushing audio buffer during pre-feed\n");
                    break;
                }
            }
            iter = iter->next;
        }
        printf("[tdksrc] Pre-fed %u audio buffers (%.1f seconds)\n", 
               audio_preload_count, (double)accumulated_duration / GST_SECOND);
    }

    if (!play_after_setup)
        goto cleanup_success;

    /* Transition to PLAYING for trickplay operations.
       With is-live=FALSE and proper appsrc configuration, state change should succeed. */
    printf("[tdksrc] Attempting state transition to PLAYING...\n");
    ret_state = gst_element_set_state(data->playbin, GST_STATE_PLAYING);
    
    if (ret_state == GST_STATE_CHANGE_FAILURE)
    {
        printf("[ERROR] Pipeline state change to PLAYING returned FAILURE\n");
        return FALSE;
    }
    
    printf("[tdksrc] State change returned: %d (0=FAILURE, 1=SUCCESS, 2=ASYNC, 3=NO_PREROLL)\n", ret_state);

    /* For ASYNC changes, wait for completion (up to 5 seconds) */
    if (ret_state == GST_STATE_CHANGE_ASYNC)
    {
        printf("[tdksrc] Waiting for async state change to complete...\n");
        timeout_ns = 5 * GST_SECOND;
        GstStateChangeReturn wait_ret = gst_element_get_state(data->playbin, &cur_state, NULL, timeout_ns);
        
        if (wait_ret == GST_STATE_CHANGE_FAILURE)
        {
            printf("[ERROR] Pipeline failed to reach PLAYING state (get_state returned FAILURE)\n");
            return FALSE;
        }
        
        printf("[tdksrc] Final state: %s (get_state returned: %d)\n", gst_element_state_get_name(cur_state), wait_ret);
        
        if (cur_state != GST_STATE_PLAYING)
        {
            printf("[ERROR] Pipeline stuck at %s state, not PLAYING\n", gst_element_state_get_name(cur_state));
            return FALSE;
        }
    }
    else
    {
        /* For SUCCESS or NO_PREROLL, state already reached */
        gst_element_get_state(data->playbin, &cur_state, NULL, GST_CLOCK_TIME_NONE);
        printf("[tdksrc] Pipeline reached state: %s\n", gst_element_state_get_name(cur_state));
    }
    
    printf("[tdksrc] Pipeline successfully in PLAYING state, ready for trickplay\n");

cleanup_success:
    g_signal_handlers_disconnect_by_func(data->playbin, (gpointer)tdkSourceSetup, &feed);
    if (feed.videoCaps != NULL)
        gst_caps_unref(feed.videoCaps);
    if (feed.audioCaps != NULL)
        gst_caps_unref(feed.audioCaps);
    
    /* Buffer reference accounting:
       - Demux stores buffers with refcount via gst_buffer_ref()
       - tdkNeedData takes extra gst_buffer_ref() before push for protection across seeks
       - For non-trickplay: buffers were pushed to pipeline, need cleanup
       - For trickplay: buffers stay in queue for future operations, keep refs alive
       - Respect only_audio and only_video flags: skip cleanup for disabled streams
       
       In non-trickplay mode: pipeline owns buffers after push, we only clean queue structure.
       In trickplay mode: skip cleanup entirely to preserve queues and refs for next operation. */
    if (!play_after_setup)
    {
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
    }
    /* Trickplay mode: queues stay alive, buffers kept safe by extra ref protection */
    
    g_free(feed.video.app.data);
    g_free(feed.audio.app.data);
    return TRUE;

cleanup_fail:
    g_signal_handlers_disconnect_by_func(data->playbin, (gpointer)tdkSourceSetup, &feed);
    if (feed.videoCaps != NULL)
        gst_caps_unref(feed.videoCaps);
    if (feed.audioCaps != NULL)
        gst_caps_unref(feed.audioCaps);
    
    /* Setup failed before pipeline creation, buffers never reached pipeline.
       Safe to cleanup and release all refs (demux ref + protection ref).
       Respect only_audio and only_video flags: skip cleanup for disabled streams */
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
    
    g_free(feed.video.app.data);
    g_free(feed.audio.app.data);
    return FALSE;
}

/********************************************************************************************************************
Purpose:               Setup stream
*********************************************************************************************************************/
static void SetupStream (MessageHandlerData *data)
{
    GstElement *playsink;
    GstStateChangeReturn state_change;
    const char* filePrefix = "file:";

    if (log_enabled)
    {
        fprintf (file,"\n########################\n");
        fprintf (file,"# Setup Pipeline Start\n");
        fprintf (file,"########################\n");
    }

    /* Create the playbin element */
    data->playbin = gst_element_factory_make(PLAYBIN_ELEMENT, NULL);
    assert_failure(data->playbin, data->playbin != NULL, "Failed to create 'playbin' element",__FUNCTION__,__LINE__,"Create Playbin Instance");
    
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
    
    /*
     * Update the current playbin flags to enable Video and Audio Playback
     */
    g_object_get (data->playbin, "flags", &flags, NULL);
    setflags();
    g_object_set (data->playbin, "flags", flags, NULL);

    /* Only set URI for standard playback (not for multiappsrc which uses tdk:// URI) */
    if (!use_multiappsrc || !tdkIsContainer(m_play_url))
    {
        g_object_set (data->playbin, "uri", m_play_url, NULL);
    }
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
    else
    {
        data->audioSink.sink = NULL;
    }
    /*
     * Link the westeros-sink to playbin (skip for audio-only playback).
     * g_object_set with "video-sink" sinks the floating ref, so the pipeline
     * becomes the sole owner.  Take an explicit extra ref here so that the
     * post-teardown gst_object_unref() in trickplayTest() is valid — matching
     * the pattern used for audioSink (obtained via g_object_get which also
     * adds a ref).
     */
    if (!only_audio)
    {
        g_object_set (data->playbin, "video-sink", data->westerosSink.sink, NULL);
        gst_object_ref (data->westerosSink.sink);
    }
    g_object_set (data->playbin, "async-handling", true, NULL);
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
    data->pipelineInitiation = true;
    data->westerosSink.previous_rendered_frames = 0;
    data->audioSink.previous_rendered_frames = 0;


    /*
     * Set playbin to PLAYING
     */

    /* Check if multiappsrc mode should be used for container files or audio-only files - do this just before playing */
    if (use_multiappsrc && (tdkIsContainer(m_play_url) || tdkIsAudioOnly(m_play_url)))
    {
        printf("\n[trickplay] Using multiappsrc mode for: %s\n", m_play_url);
        if (!setupInlineMultiAppsrc(data, true))
        {
            printf("\n[trickplay] Error: setupInlineMultiAppsrc failed\n");
            assert_failure(data->playbin, false, "Failed to setup multiappsrc mode");
        }
        DOT_GENERATE_SETUP(data->playbin);
        firstFrameReceived = false;
        data->pipelineInitiation = true;
        data->westerosSink.previous_rendered_frames = 0;
        data->audioSink.previous_rendered_frames = 0;
        return;
    }

    if (log_enabled)
    {
        fprintf (file,"\n###################################\n");
        fprintf (file,"# Setting Pipeline to PLAYING State\n");
        fprintf (file,"###################################\n");
    }

    GST_FIXME( "Setting to Playing State\n");
    assert_failure (data->playbin, gst_element_set_state (data->playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE);
    GST_FIXME( "Set to Playing State\n");
    do{
         state_change = gst_element_get_state (data->playbin, &(data->cur_state), NULL, 10000000);
    } while (state_change == GST_STATE_CHANGE_ASYNC);
    printf ("\n\n\nPipeline set to : %s  state \n\n\n", gst_element_state_get_name(data->cur_state));
    WaitForOperation;
    assert_failure (data->playbin, data->cur_state == GST_STATE_PLAYING, "Pipeline is not set to playing state", __FUNCTION__,__LINE__,"Verifying if pipeline is successfully set to PLAYING state");

    /* Generate DOT graph after pipeline setup */
    DOT_GENERATE_SETUP(data->playbin);

    /*
     * Check if the first frame received flag is set
     */
    assert_failure (data->playbin, (only_audio) || (firstFrameReceived == true), "Failed to receive first video frame signal", __FUNCTION__,__LINE__,"Verify if first frame signal is received");
    if (checkNewPlay)
        PlaybackValidation(data,5);
    else
	PlaySeconds(data->playbin,5);
    
}

/********************************************************************************************************************
 * Purpose: After a backward seek, poll video-pts and pipeline position for the time it would
 *          naturally take playback to advance from the seek target back to the pre-seek position.
 *          This guards PlaybackValidation from being entered while the position clock is still
 *          catching up after a flush-seek.
 *          For forward seeks this function returns immediately without waiting.
 * Parameters:
 *   playbin          - Pipeline element
 *   videoSink        - Westerossink element (may be NULL; used for video-pts logging)
 *   pre_seek_ns      - Pipeline position (nanoseconds) captured before the seek was issued
 *   seek_position_ns - Target position of the seek (nanoseconds)
 *   timeout_sec      - Hard cap on wait time (default 90 s)
 ********************************************************************************************************************/
static int waitUntilPreSeekPositionReached(GstElement *playbin, GstElement *videoSink,
                                            gint64 pre_seek_ns, gint64 seek_position_ns,
                                            int timeout_sec = 90)
{
    /* Only wait for backward seeks */
    if (seek_position_ns >= pre_seek_ns)
    {
        printf("\n[SEEK-WAIT] Forward seek (seek %.3f s >= pre-seek %.3f s). No wait needed.\n",
               (double)seek_position_ns / GST_SECOND, (double)pre_seek_ns / GST_SECOND);
        return 0;
    }

    /* Poll duration = natural playback time to advance from seek target to pre-seek position,
     * plus 3 extra seconds to allow the position clock to settle after the initial catch-up. */
    int wait_sec = (int)((pre_seek_ns - seek_position_ns) / GST_SECOND) + 3;
    if (wait_sec <= 0) wait_sec = 1;
    if (wait_sec > timeout_sec) wait_sec = timeout_sec;

    /* Poll every 500 ms */
    int total_polls = wait_sec * 2;

    printf("\n[SEEK-WAIT] Backward seek: polling %d s (at 500 ms intervals) for playback to advance "
           "from %.3f s to %.3f s.\n",
           wait_sec, (double)seek_position_ns / GST_SECOND, (double)pre_seek_ns / GST_SECOND);

    gint64 prev_pts = -1;
    gint64 prev_pos = -1;
    int pts_stale_count = 0;
    int pos_stale_count = 0;

    auto wait_start = std::chrono::high_resolution_clock::now();

    for (int poll = 0; poll < total_polls; poll++)
    {
        MilliSleep(500);

        gint64 cur_pos = -1;
        gst_element_query_position(playbin, GST_FORMAT_TIME, &cur_pos);

        gint64 vo_pts = 0;
        if (videoSink)
            g_object_get(videoSink, "video-pts", &vo_pts, NULL);

        /* For audio-only playback, mark pts_adv as true to skip video-pts stale counting */
        bool pts_adv = only_audio ? true : ((prev_pts >= 0) && (vo_pts > prev_pts));
        /* pos_adv requires position to advance by 400–600 ms per 500 ms poll.
         * The nominal delta is 500 ms but embedded devices exhibit ±50–100 ms of
         * OS scheduling jitter (observed range on RTK1325: 464–534 ms), so the
         * window is widened to ±100 ms from 500 ms.  Plain "cur_pos > prev_pos"
         * is still not used — it would pass even if the clock stalls then jumps. */
        gint64 pos_diff_ns = (prev_pos >= 0) ? (cur_pos - prev_pos) : 0;
        static const gint64 POS_DIFF_LO = 400000000LL;  /* 400 ms in ns */
        static const gint64 POS_DIFF_HI = 600000000LL;  /* 600 ms in ns */
        bool pos_adv = (prev_pos >= 0) && (pos_diff_ns >= POS_DIFF_LO) && (pos_diff_ns <= POS_DIFF_HI);

        const char *pts_str = only_audio ? "N/A" : ((poll == 0) ? "---" : (pts_adv ? "advancing" : "STALE"));
        const char *pos_str = (poll == 0) ? "---" : (pos_adv ? "advancing" : "STALE");

        {
            auto _ct = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm *_ts = std::localtime(&_ct);
            printf("\nTDK LOG :  %02d:%02d:%02d  --  video-pts : %lld  pts-progressing : %s  "
                   "playback-position : %.3f  position-progressing : %s",
                   _ts->tm_hour, _ts->tm_min, _ts->tm_sec,
                   (long long)vo_pts, pts_str,
                   (double)cur_pos / GST_SECOND, pos_str);

            /* Print position diff on every poll after the first */
            if (prev_pos >= 0)
            {
                printf("  pos-diff: %+.3f s (%.3f -> %.3f)%s",
                       (double)pos_diff_ns / GST_SECOND,
                       (double)prev_pos / GST_SECOND,
                       (double)cur_pos  / GST_SECOND,
                       pos_adv ? "" : "  [OUT-OF-RANGE: expected 0.400-0.600 s]");
            }
        }

        if (poll > 0)
        {
            if (!pts_adv) pts_stale_count++;
            if (!pos_adv) pos_stale_count++;
        }

        prev_pts = vo_pts;
        prev_pos = cur_pos;
    }

    int elapsed_sec = (int)std::chrono::duration_cast<std::chrono::seconds>(
                          std::chrono::high_resolution_clock::now() - wait_start).count();

    /* Summary */
    int check_polls = total_polls - 1;  /* first poll has no previous to compare */
    printf("\n[SEEK-WAIT] Post-backward-seek progress summary (%d polls checked, %d s elapsed):\n",
           check_polls, elapsed_sec);
    if (check_polls > 0)
    {
        printf("  video-pts     : %s (%d/%d polls STALE)\n",
               (pts_stale_count == 0) ? "ADVANCING" : "WARNING - NOT ADVANCING",
               pts_stale_count, check_polls);
        printf("  pipeline pos  : %s (%d/%d polls STALE)\n",
               (pos_stale_count == 0) ? "ADVANCING" : "WARNING - NOT ADVANCING",
               pos_stale_count, check_polls);
        if (pts_stale_count > 0 && pos_stale_count > 0)
            printf("  [WARN] Both video-pts and pipeline position were stale - "
                   "pipeline may not be playing after seek.\n");
        else if (pts_stale_count > 0)
            printf("  [WARN] video-pts was stale - video decoder may be lagging after seek.\n");
        else if (pos_stale_count > 0)
            printf("  [WARN] pipeline position was stale - position clock may be "
                   "catching up after seek (this can be normal for a short window).\n");
    }

    /* Overwrite the post-seek verdict globals from the wait-loop observations.
     * A metric is SUCCESS if ≤30% of polls were STALE; FAILURE if >30% were STALE.
     * Skip video-pts validation for audio-only playback (no video sink). */
    if (check_polls > 0)
    {
        int stale_threshold = (check_polls * 30) / 100;
        /* For audio-only playback, skip video-pts validation since there's no video sink */
        if (only_audio)
        {
            video_pts_validation_post_seek = "SKIP";
            printf("[SEEK-WAIT] Skipping video-pts validation (only_audio=true)\n");
        }
        else
        {
            video_pts_validation_post_seek = (pts_stale_count <= stale_threshold) ? "SUCCESS" : "FAILURE";
        }
        pipeline_position_validation_post_seek = (pos_stale_count <= stale_threshold) ? "SUCCESS" : "FAILURE";
        printf("[SEEK-WAIT] Stale threshold: %d/%d polls (30%%). "
               "pts_stale=%d pos_stale=%d\n",
               stale_threshold, check_polls, pts_stale_count, pos_stale_count);
        printf("[SEEK-WAIT] Final verdict flags (from wait loop): "
               "pipeline_position=%s  video_pts=%s\n",
               pipeline_position_validation_post_seek.c_str(),
               video_pts_validation_post_seek.c_str());
    }
    printf("[SEEK-WAIT] Wait loop elapsed: %d s. Returning to caller.\n\n", elapsed_sec);
    return elapsed_sec;
}

GST_START_TEST (trickplayTest)
{
    printf ("\n Entered into Trickplay\n");
    //file = fopen(LOG_FILE, "a");
    string operationString;
    double operationTimeout = 10.0;
    int seekSeconds = 0;
    bool seekOperation = false;
    gint64 pre_seek_position_ns = 0;  /* position (ns) before seek; passed to PlaybackValidation */
    int wait_loop_elapsed_sec = 0;    /* seconds consumed by waitUntilPreSeekPositionReached */
    gdouble rate = 1;
    GstBus *bus;
    GstStateChangeReturn state_change;
    GstState cur_state;
    MessageHandlerData data = {0};  /* Zero-initialize to ensure all members are NULL */
    int timeout = 0;
    bool is_av_playing = false;
    vector<string>::iterator operationItr;
    char* operationBuffer = NULL;

    SetupStream(&data);
    data.setRate = NORMAL_PLAYBACK_RATE;
    bus = gst_element_get_bus(data.playbin);
    guint bus_watch_id = gst_bus_add_watch(bus, (GstBusFunc) handleMessage, NULL);

    g_pipeline_destroyed = false;  /* reset before the operations loop */

    /*
     * Iterate through the list of operations recieved as input arguments and execute each of them for the requesed operation and timeperiod
     */
    for (operationItr = operationsList.begin(); operationItr != operationsList.end(); ++operationItr)
    {
        /* Guard: assert_failure() in a prior operation may have freed data.playbin
         * and set this flag. In GCheck CK_NOFORK mode fail_unless() does not
         * longjmp, so the loop would otherwise continue and dereference a freed
         * pointer.  Break early and let GCheck tally the already-recorded failure. */
        if (g_pipeline_destroyed)
        {
            printf("\n[ABORT] Pipeline destroyed by a prior assert_failure — "
                   "skipping remaining operations.\n");
            break;
        }
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
	{
		if (log_enabled)
                   fprintf(file, "\n");
		pause_operation = false;
	}

	data.currentRate = getRate(data.playbin);

	assert_failure (data.playbin, gst_element_query_position (data.playbin, GST_FORMAT_TIME, &data.currentPosition), "Failed to query the current playback position",__FUNCTION__,__LINE__,"Query playback position");

	if ((rate != data.currentRate) && (!pause_operation) && !(seekOperation && rate == 1))
	{
	    if (log_enabled)
                   fprintf(file, "\n");
	    printf("\nRequested playback rate is %f\n",rate);
	    if (rate < 0)
            {
		if (log_enabled)
                   fprintf(file, "\n");
		assert_failure (data.playbin, gst_element_query_duration (data.playbin, GST_FORMAT_TIME, &data.duration), "Failed to query the duration",__FUNCTION__,__LINE__,"Querying the duration");
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
            /* Stop processing further operations if this one failed */
            if (!operation_verdicts.empty() && !operation_verdicts.back().overall_pass)
            {
                printf("\n[ABORT] %s %.2fx failed. Skipping remaining operations.\n",
                       rate_op_type.c_str(), abs(rate_op_requested));
                break;
            }
	    if (!(rate < 0))
	    {
		if (log_enabled)
                   fprintf(file, "\n");
		if(!checkNewPlay)
		{
		    /* Playing for 20 seconds with 4x speed is equal to playing until position is 4*20 = 80 seconds */
                    operationTimeout *= abs(rate);
		}
            }
	    data.setRateOperation = FALSE;
	    WaitForOperation;
	    assert_failure (data.playbin, gst_element_query_position (data.playbin, GST_FORMAT_TIME, &startPosition), "Failed to query the current playback position",__FUNCTION__,__LINE__,"Querying the Current playback position");

	    if (latency_check_test)
            {
                latency = std::chrono::duration_cast<std::chrono::milliseconds>(time_elapsed - timestamp);
                parselatency();
            }

	}
        
    
	if (seekOperation)
	{
	    if (log_enabled)
                   fprintf(file, "\n");
	    trickplay = false;
	    data.setRateOperation = FALSE;
	    data.seekSeconds = seekSeconds;
	    /* If pipeline is not already at 1x, silently reset rate before seeking
	     * (this is an internal prerequisite, not a user-requested trickplay op) */
	    data.currentRate = getRate(data.playbin);
	    if (data.currentRate != NORMAL_PLAYBACK_RATE)
	    {
	        printf("\n[SEEK-PREP] Resetting rate from %.2f to 1.0 before seek (not recorded as operation).\n",
	               data.currentRate);
	        data.setRateOperation = TRUE;
	        data.setRate = NORMAL_PLAYBACK_RATE;
	        trickplayOperation(&data);
	        /* Remove the auto-pushed rate-reset verdict — it is not a user operation */
	        if (!operation_verdicts.empty() &&
	            operation_verdicts.back().op_type == "fastforward" &&
	            operation_verdicts.back().rate_requested == NORMAL_PLAYBACK_RATE)
	        {
	            operation_verdicts.pop_back();
	        }
	        data.setRateOperation = FALSE;
	    }
	    /* Capture position before seek by querying the pipeline directly.
	     * Do NOT use data.currentPosition here — after a silent rate reset via
	     * trickplayOperation(setRateOperation=TRUE) the field is left as
	     * GST_CLOCK_TIME_NONE because checkTrickplay only updates it in the
	     * seek (non-rate) path, which would corrupt pre_seek_position_ns. */
	    pre_seek_position_ns = 0;
	    gst_element_query_position(data.playbin, GST_FORMAT_TIME, &pre_seek_position_ns);
	    printf("\n[SEEK-PREP] pre_seek_position captured: %.3f s (seek target: %d s)\n",
	           (double)pre_seek_position_ns / GST_SECOND, seekSeconds);
	    trickplayOperation(&data);
	    startPosition = seekSeconds * (GST_SECOND);
	    if (checkNewPlay)
            {
                if (!pause_operation)
		{
		    if (log_enabled)
                       fprintf(file, "\n");
                    data.seekPosition += 1;
		}
		Sleep(1);
	    }
	    if (latency_check_test)
            {
                latency = std::chrono::duration_cast<std::chrono::milliseconds>(time_elapsed - timestamp);
                parselatency();
            }
	    /* For backward seeks, always run the wait loop regardless of checkTrickplay result.
	     * checkTrickplay only polls for 5 s right after the seek — the position clock on
	     * this platform can take much longer to catch up after a flush-seek.
	     * The wait loop is the authoritative validation for backward seeks; it overwrites
	     * pipeline_position_validation_post_seek / video_pts_validation_post_seek globals
	     * and we then update the verdict that was pushed by trickplayOperation. */
	    if (checkNewPlay && !pause_operation)
	    {
	        wait_loop_elapsed_sec = waitUntilPreSeekPositionReached(
	                                        data.playbin, data.westerosSink.sink,
	                                        pre_seek_position_ns,
	                                        (gint64)seekSeconds * GST_SECOND);

	        /* Update the seek verdict with wait-loop results for backward seeks */
	        if (!operation_verdicts.empty() && operation_verdicts.back().op_type == "seek")
	        {
	            OperationVerdict& sv = operation_verdicts.back();
	            sv.pos_result   = pipeline_position_validation_post_seek;
	            sv.pts_result   = video_pts_validation_post_seek;
	            sv.overall_pass = (pipeline_position_validation_post_seek == "SUCCESS");
	            printf("\n[SEEK-VERDICT] Updated after wait loop: "
	                   "pipeline_position=%s  video_pts=%s  overall=%s\n",
	                   sv.pos_result.c_str(), sv.pts_result.c_str(),
	                   sv.overall_pass ? "PASS" : "FAIL");
	        }
	    }
	    else if (pause_operation)
	    {
	        /* Pipeline is in PAUSE state (from a preceding pause operation).
	         * The wait loop is skipped intentionally: position will not advance
	         * while paused, so polling would mark all samples STALE and
	         * incorrectly fail the seek verdict.
	         * The seek verdict from checkTrickplay (position/pts at seek target)
	         * is used as-is. */
	        printf("\n[SEEK-WAIT] Skipped: pipeline is PAUSED after seek "
	               "(pause_operation=true). Position advancement cannot be "
	               "validated in PAUSED state.\n");
	    }

            /* Abort on failure only after the wait loop has had its say */
            if (!operation_verdicts.empty() && !operation_verdicts.back().overall_pass)
            {
                printf("\n[ABORT] Seek to %d s failed. Skipping remaining operations.\n", seekSeconds);
                break;
            }
	}

	if ("play" == operationString)
        {
	    if (log_enabled)
               fprintf(file, "\n");
	    trickplay = false;
	    /*
             * If pipeline is already in playing state with normal playback rate (1.0),
	     * just wait for operationTimeout seconds, instead os setting the pipeline to playing state again
	     */	
	    fail_unless_equals_int (gst_element_get_state (data.playbin, &cur_state,
                                                                  NULL, 0), GST_STATE_CHANGE_SUCCESS);
	    if ((cur_state != GST_STATE_PLAYING))
	    {
		 if (log_enabled)
                   fprintf(file, "\n");
              	 /* Set the playbin state to GST_STATE_PLAYING
                  */
		 assert_failure (data.playbin, gst_element_set_state (data.playbin, GST_STATE_PLAYING) !=  GST_STATE_CHANGE_FAILURE, "Failed to set to PLAYING state",
                        __FUNCTION__,__LINE__,"Setting pipeline to PLAYING state");

		 do{
		 //Waiting for state change
                    /*
                     * Polling for the state change to reflect with 10 ms timeout
                     */
                    state_change = gst_element_get_state (data.playbin, &cur_state, NULL, 10000000);
                 } while (state_change == GST_STATE_CHANGE_ASYNC);
		 printf ("\n********Current state is: %s \n", gst_element_state_get_name(cur_state));
             }
	     assert_failure (data.playbin, gst_element_query_position (data.playbin, GST_FORMAT_TIME, &startPosition), "Failed to query the current playback position",__FUNCTION__,__LINE__,"Querying the Current Playback position");

             /*
	      * Wait for the requested time
	      */
             printf ("Waiting for %f seconds\n", operationTimeout);

             /* Record play operation verdict */
             {
                 OperationVerdict _pv;
                 _pv.op_type     = "play";
                 _pv.overall_pass = true;
                 operation_verdicts.push_back(_pv);
             }
	 } 
	 else if ("pause" == operationString)
         {
	     if (log_enabled)
                 fprintf(file, "\n");
	     trickplay = false;
	     pause_operation = true;
             /*
	      * Set the playbin state to GST_STATE_PAUSED
	      */	
	     assert_failure (data.playbin, gst_element_query_position (data.playbin, GST_FORMAT_TIME, &startPosition), "Failed to query the current playback position",__FUNCTION__,__LINE__,"Querying the Current Playback position");

	     gst_element_set_state (data.playbin, GST_STATE_PAUSED);
	     do{
                 //Waiting for state change
                 state_change = gst_element_get_state (data.playbin, &cur_state, NULL, 10000000);
             } while (state_change == GST_STATE_CHANGE_ASYNC);
	     assert_failure (data.playbin, gst_element_get_state (data.playbin, &cur_state, NULL, 0) == GST_STATE_CHANGE_SUCCESS, "Failed to get playback state",
                    __FUNCTION__,__LINE__,"Obtain pipeline state");

	     assert_failure (data.playbin, cur_state == GST_STATE_PAUSED, "Pipeline is not set to PAUSED state", __FUNCTION__,__LINE__, "Verify Pipeline is set to PAUSED state");
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

	     /* Record pause operation verdict */
	     {
	         OperationVerdict _pv;
	         _pv.op_type     = "pause";
	         _pv.overall_pass = true;
	         operation_verdicts.push_back(_pv);
	     }
	 }
	 if (true == checkAVStatus)
	 {    
	     is_av_playing = check_for_AV_status();
	     assert_failure (data.playbin,is_av_playing == true, "Video is not playing in TV",
                        __FUNCTION__,__LINE__, "Verify video playback using proc entry");
             printf ("DETAILS: SUCCESS, Video playing successfully \n");
 	 }

	 timeout=operationTimeout;
	 if (checkNewPlay)
	 {  
	     if (data.setRate != NORMAL_PLAYBACK_RATE)
	     {
		 printf( "\nSetRate Operation was invoked\n");
		 if (log_enabled)
                    fprintf(file, "\nSetRate Operation was invoked\n");
                 data.pipelineInitiation = true;
		 //Reset setRate
		 data.setRate = NORMAL_PLAYBACK_RATE;
	     }
             /* For backward seeks the wait loop ran gap+3 s, so the pipeline has
              * already advanced past pre_seek_position_ns by the time we get here.
              * Query the actual current position and use that as the PlaybackValidation
              * baseline so the first poll diff is near zero.
              * For forward seeks pass 0 to let PlaybackValidation use seekPosition as usual.
              * Only do this when the wait loop actually ran (wait_loop_elapsed_sec > 0).
              * If the wait loop was skipped (e.g. pause_operation=true) the pipeline is still
              * at the seek target, so pass 0 and let PlaybackValidation use seekPosition. */
             gint64 playback_baseline_ns = 0;
             if (pre_seek_position_ns > (gint64)seekSeconds * (gint64)GST_SECOND
                 && wait_loop_elapsed_sec > 0)
             {
                 /* Backward seek — wait loop ran; query actual current position */
                 gst_element_query_position(data.playbin, GST_FORMAT_TIME, &playback_baseline_ns);
                 if (playback_baseline_ns <= 0)
                     playback_baseline_ns = pre_seek_position_ns;
                 printf("\n[SEEK-BASELINE] Backward seek: using current position %.3f s as PlaybackValidation baseline\n",
                        (double)playback_baseline_ns / GST_SECOND);
             }
             else if (pre_seek_position_ns > (gint64)seekSeconds * (gint64)GST_SECOND
                      && wait_loop_elapsed_sec == 0)
             {
                 /* Backward seek — wait loop skipped (pause); use seek target */
                 printf("\n[SEEK-BASELINE] Wait loop was skipped; using seek target %d s as PlaybackValidation baseline\n",
                        seekSeconds);
             }
             else if (seekOperation)
             {
                 /* Forward seek — SEEK-CHECK + Sleep(1) consumed variable wall time,
                  * so the pipeline has already advanced past seekSeconds+1 by the time
                  * PlaybackValidation starts. Query the actual current position as baseline
                  * instead of the stale (seekSeconds+1) value to avoid a constant diff
                  * that grows into a false failure. */
                 gst_element_query_position(data.playbin, GST_FORMAT_TIME, &playback_baseline_ns);
                 if (playback_baseline_ns <= 0)
                     playback_baseline_ns = (gint64)seekSeconds * (gint64)GST_SECOND;
                 printf("\n[SEEK-BASELINE] Forward seek: using current position %.3f s as PlaybackValidation baseline\n",
                        (double)playback_baseline_ns / GST_SECOND);
             }
             /* Adjust timeout: subtract time already spent in the wait loop */
             int adjusted_timeout = timeout - wait_loop_elapsed_sec;
             wait_loop_elapsed_sec = 0;  /* reset for next iteration */
             if (adjusted_timeout <= 0)
             {
                 printf("\n[SEEK-WAIT] Wait loop consumed all %d s of play timeout. "
                        "Skipping PlaybackValidation.\n", timeout);
             }
             else
             {
                 if (adjusted_timeout < timeout)
                     printf("\n[SEEK-WAIT] %d s consumed by wait loop; running "
                            "PlaybackValidation for remaining %d s.\n",
                            timeout - adjusted_timeout, adjusted_timeout);
                 PlaybackValidation(&data, adjusted_timeout, seekOperation, playback_baseline_ns);
             }
	 }
	 else
         {
             PlaySeconds(data.playbin,timeout,seekOperation);
	 }
	 seekOperation = false;
	 free(operationBuffer);  /* Free the strdup'd operation string */
    }

    /* Send EOS to multiappsrc streams after all operations are complete */
    if (multiappsrc_defer_eos && (multiappsrc_video_appsrc != NULL || multiappsrc_audio_appsrc != NULL))
    {
        printf("\n[trickplay] Sending deferred EOS to multiappsrc after all operations complete\n");
        GstFlowReturn ret;
        if (multiappsrc_video_appsrc != NULL)
        {
            printf("[trickplay] Emitting EOS to video appsrc\n");
            g_signal_emit_by_name(multiappsrc_video_appsrc, "end-of-stream", &ret);
        }
        if (multiappsrc_audio_appsrc != NULL)
        {
            printf("[trickplay] Emitting EOS to audio appsrc\n");
            g_signal_emit_by_name(multiappsrc_audio_appsrc, "end-of-stream", &ret);
        }
        multiappsrc_defer_eos = false;  /* Reset flag for next test */
    }

    printf("\n unref the bus\n");

    /* Remove bus watch source before terminating pipeline */
    g_source_remove(bus_watch_id);

    /* Generate DOT graph before pipeline termination */
    DOT_GENERATE_FINAL(data.playbin);

    terminatePipeline(data.playbin);
    data.playbin = NULL;  /* prevent use-after-free; terminatePipeline already freed it */

    /* Unref bus AFTER pipeline termination to avoid use-after-free */
    gst_object_unref(bus);
    bus = NULL;

    /* Reset multiappsrc globals after pipeline termination */
    multiappsrc_video_appsrc = NULL;
    multiappsrc_audio_appsrc = NULL;
    multiappsrc_defer_eos = false;

    /* Drop the original factory_make references to the sink elements.
     * playbin held one ref to each during playback; that ref was released when
     * playbin was destroyed by terminatePipeline() above.  The initial ref
     * from gst_element_factory_make() is still outstanding and must be released
     * explicitly — otherwise the GStreamer elements are leaked. */
    if (data.westerosSink.sink)
    {
        gst_object_unref(data.westerosSink.sink);
        data.westerosSink.sink = NULL;
    }
    if (data.audioSink.sink)
    {
        gst_object_unref(data.audioSink.sink);
        data.audioSink.sink = NULL;
    }

    /* ================================================================
     * POST-TEARDOWN FINAL VERDICT FOR ALL TRICKPLAY OPERATIONS
     * Verdicts are printed in execution order.
     * The first failed operation causes the testcase to FAIL.
     * ================================================================ */
    if (!operation_verdicts.empty())
    {
        bool any_failed = printOperationVerdicts();
        fail_unless(!any_failed,
                    "One or more trickplay operations FAILED. "
                    "See TRICKPLAY OPERATIONS - VERDICT SUMMARY above.");
    }
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
	    if (log_enabled)
                fprintf(file, "Error setting environment variable: %s\n", varName);
            printf("Error setting environment variable: %s\n", varName);
        }
	else
	{
	    if (log_enabled)
                fprintf(file, "Set environment variable: %s=%s\n", varName, varValue);
            printf("Set environment variable: %s=%s\n", varName, varValue);
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
            // Access the 'state' field in the first object in the result array (RDKShell/RDKWindowManager format)
            if (root["result"][0].isObject() && root["result"][0].isMember("state"))
            {
                std::string state = root["result"][0]["state"].asString();
	        printf(" %s \n",state.c_str());
                return state == "activated";
            }
            // Check for clients array (RDKShell format)
            if (root["result"].size() > 0 && root["result"][0].isString())
            {
                std::string client = root["result"][0].asString();
                return client == "test";
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
            // Check for "clients" array (RDKShell format)
            else if (root["result"].isMember("clients") && root["result"]["clients"].isArray())
            {
                if (root["result"]["clients"].size() > 0)
                {
                    std::string client = root["result"]["clients"][0].asString();
                    return client == "test";
                }
                return false;
            }
            // Check for "apps" array (RDKWindowManager format)
            else if (root["result"].isMember("apps") && root["result"]["apps"].isArray())
            {
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
 * Purpose      : Function to obtain RDKShell/RDKWindowManager plugin status
 ********************************************************************************************************************/
bool getPluginStatus()
{
    std::string jsonData;
    if (useWindowManager) {
        jsonData = R"({"jsonrpc": "2.0", "id": 2, "method": "Controller.1.status@org.rdk.RDKWindowManager"})";
    } else {
        jsonData = R"({"jsonrpc": "2.0", "id": 2, "method": "Controller.1.status@org.rdk.RDKShell"})";
    }
    // Call sendRequest function to get the JSON response as string
    std::string response = sendRequest(jsonData);
    if (response == "FAILURE")
	printf ("\nERROR : Unable to send request to 0.0.0.0:9998\n");
    else
	printf(useWindowManager ? "\nRDKWindowManager State : " : "\nRDKShell State : ");
    return parseResult(response);
}

/********************************************************************************************************************
 * Purpose      : Function to activate display manager plugin (RDKShell/RDKWindowManager)
 ********************************************************************************************************************/
bool activatePlugin()
{
    std::string jsonData;
    if (useWindowManager) {
        jsonData = R"({"jsonrpc": "2.0", "id": 2, "method": "Controller.1.activate", "params": { "callsign": "org.rdk.RDKWindowManager" }})";
    } else {
        jsonData = R"({"jsonrpc": "2.0", "id": 2, "method": "Controller.1.activate", "params": { "callsign": "org.rdk.RDKShell" }})";
    }
    // Call sendRequest function to get the JSON response as string
    std::string response = sendRequest(jsonData);
    return parseResult(response);
}

/********************************************************************************************************************
 * Purpose      : Function to verify if a display client named test is already running
 ********************************************************************************************************************/
bool checkDisplayClient()
{
    std::string jsonData;
    if (useWindowManager) {
        jsonData = R"({
            "jsonrpc": "2.0",
            "id": 1,
            "method": "org.rdk.RDKWindowManager.1.getApps"
        })";
    } else {
        jsonData = R"({
            "jsonrpc": "2.0",
            "id": 1,
            "method": "org.rdk.RDKShell.1.getClients"
        })";
    }

    // Call sendRequest function to get the JSON response as string
    std::string response = sendRequest(jsonData);
    return parseResult(response);
}

/********************************************************************************************************************
 * Purpose      : Function to create display client via display manager plugin
 ********************************************************************************************************************/
bool createDisplayClient()
{
    std::string jsonData;
    if (useWindowManager) {
        jsonData = R"({
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
    } else {
        jsonData = R"({
            "jsonrpc": "2.0",
            "id": 1,
            "method": "org.rdk.RDKShell.1.createDisplay",
            "params": {
                "client": "test",
                "displayName": "test"
            }
        })";
    }

    // Call sendRequest function to get the JSON response as string
    std::string response = sendRequest(jsonData);
    return parseResult(response);
}

/********************************************************************************************************************
 * Purpose      : Function to destroy display client via display manager plugin
 ********************************************************************************************************************/
bool destroyDisplayClient()
{
    std::string jsonData;
    if (useWindowManager) {
        printf("\nDestroying display client via RDKWindowManager plugin is not implemented\n");
        return true;
    } else {
        jsonData = R"({
            "jsonrpc": "2.0",
            "id": 1,
            "method": "org.rdk.RDKShell.1.kill",
            "params": {
                "client": "test"
            }
        })";
    }

    // Call sendRequest function to get the JSON response as string
    std::string response = sendRequest(jsonData);
    return parseResult(response);
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
            libHandles.push_back(libPath);
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
        fprintf (file,"# Command invoked -> \n");
        for (int i=0; i <argc; i++)
        {
            fprintf (file," %s ", argv[i]);
        }
	fprintf(file,"\n%s\n\n",std::string(188,'#').c_str());
    }
}

void execute_postrequisite ()
{
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
        printf(useWindowManager ? "Destroying RDKWindowManager Display" : "Destroying RDKShell Display");
        if (log_enabled)
            fprintf(file, useWindowManager ? "Destroying RDKWindowManager Display" : "Destroying RDKShell Display");
        if (!destroyDisplayClient())
        {
            printf(useWindowManager ? "ERROR: Unable to destroy RDKWindowManager display\n" : "ERROR: Unable to destroy RDKShell display\n");
            if (log_enabled)
                fprintf(file, useWindowManager ? "ERROR: Unable to destroy RDKWindowManager display\n" : "ERROR: Unable to destroy RDKShell display\n");
            //Add destroying display failure to number of failures
            returnValue += 1;
        }
    }
    closeLibs();
    if (log_enabled)
        fprintf(file , "\n################## End of Execution ##############################\n");
    if (file)
       fclose(file);
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
    file = fopen(LOG_FILE, "w");

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
            
            /* If first operation is not "play", prepend "play:5" to ensure pipeline reaches PLAYING state
               This fixes standalone fastforward/rewind operations which fail without prior play. */
            if (!operationsList.empty())
            {
                std::string firstOp = operationsList[0];
                if (firstOp.find("play") != 0)  /* Doesn't start with "play" */
                {
                    printf("[trickplay-startup] First operation is not 'play' (%s), prepending 'play:5'\n", firstOp.c_str());
                    operationsList.insert(operationsList.begin(), "play:5");
                }
            }
            
            /* Calculate buffer duration for trickplay operations with multiappsrc */
            if (use_multiappsrc && !operationsList.empty())
            {
                trickplay_buffer_duration_ns = calculateBufferDurationForTrickplay();
            }
         }
         if (strcmp ("use_multiappsrc=yes", argv[arg]) == 0)
         {
            use_multiappsrc = true;
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
	    use_westerossink_fps = false;
         }
	 if (strcmp ("checkAudioFPS=no", argv[arg]) == 0)
         {
	    use_audioSink = false;
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
	 if (strcmp ("checkEachSecondPTS", argv[arg]) == 0)
         {
            checkEachSecondPTS = true;
         }
	 if (strcmp ("only_audio", argv[arg]) == 0)
	 {
            only_audio = true;  		
	 }
	 if (strcmp ("only_audio=yes", argv[arg]) == 0)
	 {
            only_audio = true;  		
	 }
	 if (strcmp ("only_video", argv[arg]) == 0)
	 {
            only_video = true;
	 }
	 if (strcmp ("only_video=yes", argv[arg]) == 0)
	 {
            only_video = true;
	 }
	 if (strcmp ("startWesteros=yes", argv[arg]) == 0)
         {
            startWesterosConfig = true;
	    defaultStart = false;
         }
	 if (strcmp ("createDisplay=yes", argv[arg]) == 0)
         {
            createDisplayConfig = true;
            startWesterosConfig = false;
	    defaultStart = false;
         }

    }
    gst_check_init (&argc, &argv);
    
    if (defaultStart)
    {
        bool RDKWindowManager_exists = fileExists("/etc/WPEFramework/plugins/RDKWindowManager.json", true);
        bool RDKShell_exists = fileExists("/etc/WPEFramework/plugins/RDKShell.json", true);
	if (RDKWindowManager_exists)
	{
	    useWindowManager = true;
	    printf("\nRDKWindowManager is present in device\nCreating display - 'test'\n");
	    if (log_enabled)
		fprintf(file, "\nRDKWindowManager is present in device\nCreating display - 'test'\n");
	    createDisplayConfig = true;
            startWesterosConfig = false;
	}
	else if (RDKShell_exists)
	{
	    useWindowManager = false;
	    printf("\nRDKShell is present in device\nCreating display - 'test'\n");
	    if (log_enabled)
		fprintf(file, "\nRDKShell is present in device\nCreating display - 'test'\n");
	    createDisplayConfig = true;
            startWesterosConfig = false;
	}
	else
	{
	    printf("\nRDKWindowManager/RDKShell is not present in device\nProceeding to create display using westeros renderer\n");
	    if (log_enabled)
		fprintf(file, "\nRDKWindowManager/RDKShell is not present in device\nProceeding to create display using westeros renderer\n");
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
             printf(useWindowManager ? "\nRDKWindowManager is deactivated\n" : "\nRDKShell is deactivated\n");
	     fprintf(file, useWindowManager ? "\nRDKWindowManager is deactivated\n" : "\nRDKShell is deactivated\n");
             printf(useWindowManager ? "Activating RDKWindowManager\n" : "Activating RDKShell\n");
	     fprintf(file, useWindowManager ? "Activating RDKWindowManager\n" : "Activating RDKShell\n");
             if (activatePlugin())
             {
                  printf(useWindowManager ? "\nActivate RDKWindowManager success\n" : "\nActivate RDKShell success\n");

                  //Wait for plugin to activate
                  sleep(5);
                  if (!getPluginStatus())
                  {
                       printf(useWindowManager ? "\nERROR : Unable to activate RDKWindowManager plugin" : "\nERROR : Unable to activate RDKShell plugin");
		       fprintf(file, useWindowManager ? "\nERROR : Unable to activate RDKWindowManager plugin" : "\nERROR : Unable to activate RDKShell plugin");
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
             printf(useWindowManager ? "\nCreating RDKWindowManager Display" : "\nCreating RDKShell Display");
	     if (log_enabled)
	         fprintf(file, useWindowManager ? "\nCreating RDKWindowManager Display" : "\nCreating RDKShell Display");
             if (!createDisplayClient())
             {
                  printf("\nERROR : Unable to create Display\n");
		  if (log_enabled)
	              fprintf(file, "\nERROR : Unable to create Display\n");
                  goto exit;
             }
             else
             {
                  displayCreated = true;
		  printf(useWindowManager ? "\nSUCCESS : RDKWindowManager display created successfully\n" : "\nSUCCESS : RDKShell display created successfully\n");
		  if (log_enabled)
                      fprintf(file, useWindowManager ? "\nSUCCESS : RDKWindowManager display created successfully\n" : "\nSUCCESS : RDKShell display created successfully\n");
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
    //fclose(file);

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

    execute_postrequisite();
    return returnValue;
}
