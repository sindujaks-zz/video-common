// Copyright (c) 2019 Intel Corporation.  //
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @brief UDF loading example
 */

#include <cstring>
#include <thread>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <eii/utils/logger.h>
#include <eii/utils/json_config.h>
#include <eii/msgbus/msgbus.h>
#include <opencv2/opencv.hpp>
#include "eii/udf/udf_manager.h"
#include "eii/udf/frame.h"

#define SERVICE_NAME "load-example"

using namespace eii::udf;
using namespace eii::msgbus;

/**
 * Free method to free the underlying @c cv::Mat wrapped by the udf::Frame
 * object.
 */
void free_cv_frame(void* frame) {
    LOG_DEBUG_0("Freeing load-example frame");
    cv::Mat* mat = (cv::Mat*) frame;
    delete mat;
}

int main(int argc, char** argv) {
    try {
        set_log_level(LOG_LVL_DEBUG);
        config_t* config = json_config_new("config.json");
        config_t* msgbus_config = json_config_new("msgbus_config.json");
        config_t* sub_config = json_config_new("msgbus_config.json");

        LOG_INFO_0("Initializing queues");
        FrameQueue* input_queue = new FrameQueue(-1);
        FrameQueue* output_queue = new FrameQueue(-1);
        FrameQueue* sub_queue = new FrameQueue(-1);

        LOG_INFO_0("Initializing UDFManager");
        UdfManager* manager = new UdfManager(
                config, sub_queue, output_queue, SERVICE_NAME);
        manager->start();

        LOG_INFO_0("Initializing Publisher thread");
        std::condition_variable err_cv;
        Publisher* publisher = new Publisher(
                msgbus_config, err_cv, "example", (MessageQueue*) input_queue,
                SERVICE_NAME);
        publisher->start();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        Subscriber<Frame>* subscriber = new Subscriber<Frame>(
                sub_config, err_cv, "example", (MessageQueue*) sub_queue,
                SERVICE_NAME);
        subscriber->start();

        LOG_INFO_0("Adding frames to input queue");

        // Load OpenCV frame
        cv::Mat* cv_frame = new cv::Mat();
        cv::Mat* cv_frame2 = new cv::Mat();

        *cv_frame = cv::imread("load_example_frame.png");
        *cv_frame2 = cv_frame->clone();

        Frame* frame = new Frame(
                (void*) cv_frame,  free_cv_frame, cv_frame->data,
                cv_frame->cols, cv_frame->rows, cv_frame->channels(),
                EncodeType::JPEG, 50);

        frame->add_frame((void*) cv_frame2,  free_cv_frame, cv_frame2->data,
                cv_frame2->cols, cv_frame2->rows, cv_frame2->channels(),
                EncodeType::JPEG, 50);

        input_queue->push(frame);

        LOG_INFO_0("Waiting for processed frame...");
        output_queue->wait();
        Frame* processed = output_queue->front();
        output_queue->pop();

        // ---- Uncomment to save processed frame

        //int w = processed->get_width();
        //int h = processed->get_height();
        //int c = processed->get_channels();

        //cv::Mat mat_frame(h, w, CV_8UC(c), processed->get_data());
        //cv::imwrite("received.jpg", mat_frame);

        //----

        // Free processed frame
        delete processed;

        LOG_INFO_0("Stopping subscriber");
        subscriber->stop();
        delete subscriber;

        LOG_INFO_0("Cleaning up publisher");
        delete publisher;

        LOG_INFO_0("Cleaning up UDFManager");
        delete manager;
        delete input_queue;
    } catch(const char* ex) {
        LOG_INFO("Failed to load exception: %s", ex);
        return -1;
    }

    return 0;
}
