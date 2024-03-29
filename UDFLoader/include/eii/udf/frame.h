// Copyright (c) 2019 Intel Corporation.
//
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
 * @file
 * @brief High level video frame abstraction.
 */

#ifndef _EII_UDF_FRAME_H
#define _EII_UDF_FRAME_H

#include <atomic>
#include <string>
#include <vector>

#include <eii/msgbus/msg_envelope.h>
#include <eii/utils/logger.h>

namespace eii {
namespace udf {

/**
 * Encoding types for the given frame.
 */
enum EncodeType {
    NONE,
    JPEG,
    PNG,
};

/**
 * Representation of basic frame meta-data.
 *
 * \note This is an internal class, not used outside of the Frame object.
 */
class FrameMetaData {
private:
    std::string m_img_handle;
    int m_width;
    int m_height;
    int m_channels;
    EncodeType m_encode_type;
    int m_encode_level;

    /**
     * Private @c FrameMetaData copy constructor.
     */
    FrameMetaData(const FrameMetaData& src);

    /**
     * Private @c FrameMetaData assignment operator.
     */
    FrameMetaData& operator=(const FrameMetaData& src);

public:
    /**
     * Constructor
     *
     * TODO(kmidkiff): Document this code
     */
    FrameMetaData(
            std::string img_handle, int width, int height, int channels,
            EncodeType encode_type, int encode_level);

    /**
     * Destructor
     */
    ~FrameMetaData();

    // Setters
    void set_width(int width);
    void set_height(int height);
    void set_channels(int channels);
    void set_encoding(EncodeType encode_type, int encode_level);

    // Getters
    std::string get_img_handle();
    int get_width();
    int get_height();
    int get_channels();
    EncodeType get_encode_type();
    int get_encode_level();
};

/**
 * Holder for underlying data of a frame.
 *
 * \note This is an internal class, not used outside of the Frame object.
 */
class FrameData {
private:
    FrameMetaData* m_meta;
    void* m_frame;
    void* m_data;
    void (*m_free_frame)(void*);
    size_t m_size;

    /**
     * Private @c FrameData copy constructor.
     */
    FrameData(const FrameData& src);

    /**
     * Private @c FrameData assignment operator.
     */
    FrameData& operator=(const FrameData& src);

public:
    // TODO(kmidkiff): Document this more
    FrameData(
            void* frame, void (*free_frame)(void*), void* data,
            FrameMetaData* meta);

    ~FrameData();

    FrameMetaData* get_meta_data();
    void* get_data();
    size_t get_size();

    /**
     * Encode the underlying frame.
     *
     * CAUTION: This changes underlying data and frees the old data. This
     * action is irreversable.
     */
    void encode();
};

/**
 * Wrapper around a frame object
 */
class Frame : public eii::msgbus::Serializable {
private:
    // This pointer represents the underlying object in which the raw pixel
    // data for the frame resides. This pointer could be a GstBuffer, cv::Mat
    // object, or any other representation. The purpose of having this pointer
    // is to keep the memory of the underlying frame alive while the user needs
    // to access the underlying bytes for the frame.
    // void* m_frame;

    // Underlying free method for the frame
    // void (*m_free_frame)(void*);

    // Total number of frames
    // int m_num_frames;

    // This pointer points to the underlying bytes for the frame, i.e. the
    // bytes for the raw pixels of the frame. Note that the memory for this
    // void* is ultimatly residing in the m_frame's memory, this is just a
    // pointer to that underlying data provided to the constructor.
    // void** m_data;
    std::vector<FrameData*> m_frames;

    // This is only used when the frame was deserialized from a
    // msg_envelope_t and then reserialized. This keeps track of the actual
    // underlying blob memory (i.e. the frame's pixel data).
    // owned_blob_t** m_blob_ptr;

    // Meta-data associated with the frame
    msg_envelope_t* m_meta_data;
    // Additional frames array in the meta-data (if it exists)
    msg_envelope_elem_body_t* m_additional_frames_arr;

    // Must-have attributes
    // int m_width;
    // int m_height;
    // int m_channels;

    // Flag for if the frame has been serailized already
    std::atomic<bool> m_serialized;

    // Encoding type for the frame
    // EncodeType m_encode_type;

    // Encoding level for the frame
    // int m_encode_level;

    /**
     * Private helper function to encode the given frame during serialization.
     */
    // void encode_frame();

    /**
     * Function to be passed to the EII Message Bus for freeing the frame after
     * it has been transmitted over the bus.
     */
    static void msg_free_frame(void* hint) {
        LOG_DEBUG_0("Freeing frame...");
        if (hint == NULL) {
            LOG_ERROR_0("Returning because frame is NULL...");
            return;
        }

        // Cast to a frame pointer
        Frame* frame = (Frame*) hint;
        delete frame;
    };

    /**
     * Private @c Frame copy constructor.
     */
    Frame(const Frame& src);

    /**
     * Private @c Frame assignment operator.
     */
    Frame& operator=(const Frame& src);

public:
    /**
     * Constructor for representing a single frame.
     *
     * @param frame             - Underlying frame object
     * @param free_frame        - Function to free the underlying frame
     * @param data              - Constant pointer to the underlying frame data
     * @param width             - Frame width
     * @param height            - Frame height
     * @param channels          - Number of channels in the frame
     * @param encode            - (Optional) Frame encoding type
     *                            (default:  @c EncodeType:NONE)
     * @param encode_level      - (Optional) Encode level
     *                            (value depends on encoding type)
     */
    Frame(void* frame, void (*free_frame)(void*), void* data,
          int width, int height, int channels,
          EncodeType encode_type=EncodeType::NONE, int encode_level=0);

    /**
     * Initialize an empty frame.
     */
    Frame();

    /**
     * Deserialize constructor
     *
     * @param msg - Message envelope to deserialize
     */
    Frame(msg_envelope_t* msg);

    /**
     * Destructor
     */
    ~Frame();

    /**
     * Get frame encoding type
     *
     * @param index - Index of the internal frame (default: 0)
     * @return EncodeType
     */
    EncodeType get_encode_type(int index=0);

    /**
     * Get the image handle for the frame.
     *
     * @param index - Index of the internal frame (default: 0)
     * @return std::string
     */
    std::string get_img_handle(int index=0);

    /**
     * Get frame encoding level
     *
     * @param index - Index of the internal frame (default: 0)
     * @return int
     */
    int get_encode_level(int index = 0);

    /**
     * Get frame width.
     *
     * @param index - Index of the internal frame (default: 0)
     * @return int
     */
    int get_width(int index=0);

    /**
     * Get frame height.
     *
     * @param index - Index of the internal frame (default: 0)
     * @return int
     */
    int get_height(int index=0);

    /**
     * Get number of channels in the frame.
     *
     * @param index - Index of the internal frame (default: 0)
     * @return int
     */
    int get_channels(int index=0);

    /**
     * Get the underlying frame data.
     *
     * @param index - Index of the internal frame (default: 0)
     * @return void* */
    void* get_data(int index=0);

    /**
     * Get the number of frames in Frame object.
     *
     * @return void*
     */
    int get_number_of_frames();

    /**
     * Add another underlying frame for the Frame object to track.
     *
     * @param frame             - Underlying frame object
     * @param free_frame        - Function to free the underlying frame
     * @param data              - Constant pointer to the underlying frame data
     * @param width             - Frame width
     * @param height            - Frame height
     * @param channels          - Number of channels in the frame
     * @param encode            - (Optional) Frame encoding type
     *                            (default:  @c EncodeType:NONE)
     * @param encode_level      - (Optional) Encode level
     *                            (value depends on encoding type)
     */
    void add_frame(
            void* frame, void (*free_frame)(void*), void* data,
            int width, int height, int channels,
            EncodeType encode_type=EncodeType::NONE, int encode_level=0);

    /**
     * Modify data for a frame.
     *
     * @param index             - Index of frame to be set
     * @param frame             - Underlying frame object
     * @param free_frame        - Function to free the underlying frame
     * @param data              - Constant pointer to the underlying frame data
     * @param width             - Frame width
     * @param height            - Frame height
     */
    void set_data(
            int index, void* frame, void (*free_frame)(void*), void* data,
            int width, int height, int channels);

    /**
     * Set the encoding for the frame.
     *
     * @param enc_type - Encoding type
     * @param enc_lvl  - Encoding level
     * @param index    - Index of frame to be set (df: index 0)
     */
    void set_encoding(EncodeType enc_type, int enc_lvl, int index=0);

    /**
     * Get @c msg_envelope_t meta-data envelope.
     *
     * \note NULL will be returned if the frame has already been serialized.
     *
     * @return @c msg_envelope_t*
     */
    msg_envelope_t* get_meta_data();

    /**
     * \note **IMPORTANT NOTE:** This method PERMANENTLY changes the frame
     *      object and can only be called ONCE. The reson for this is to make
     *      sure that all of the underlying memory is properly managed and
     *      free'ed at a point where the application will not SEGFAULT due to a
     *      double free situation. All methods (except for width(), height(),
     *      and channels()) will return errors after this.
     *
     * Overriden serialize method.
     *
     * @return @c msg_envelope_t*
     */
    msg_envelope_t* serialize() override;
};

} // udf
} // eii

#endif // _EII_UDF_FRAME_H
