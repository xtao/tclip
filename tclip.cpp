extern "C" {
    #include <Python.h>
}

#include "cv.h"
#include "opencv2/core/core.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"

#include <iostream>
#include <map>
#include <math.h>
#include <time.h>

using namespace cv;
using namespace std;

static CascadeClassifier face_cascade;

int
detectFace(Mat &img){
    std::vector<Rect> faces;
    Mat img_gray;
    int face_size;
    int Y;

    cvtColor( img, img_gray, CV_BGR2GRAY );
    equalizeHist( img_gray, img_gray );
    face_cascade.detectMultiScale( img_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );
    face_size = faces.size();

    if (face_size > 0) {
        Y = faces[face_size -1].y - faces[face_size -1].height / 2;
        //fix
        if (Y > img.size().height / 2) {
            return -1;
        } else {
            return Y < 0 ? 0 : Y;
        }
    } else {
        return -1;
    }
}

int
detectCharacter(Mat &img){
    int start_x = 0; //特征点X坐标开始位置
    int end_x = 0; //特征点X坐标结束位置
    int section_index = 0; //Y坐标段数字索引
    map<int,int> section_num; //每个Y坐标段中特征点的数量
    int total = 0; //总共特征点数量
    int avg = 0; //每个Y坐标段的平均特征点数量
    int con_num = 4; //需要连续的阀值
    int flag = 0;
    int counter = 0;
    int Y = 0;

    vector<KeyPoint> keypoints;

    cv::initModule_nonfree();//使用SIFT/SURF create之前，必须先initModule_<modulename>();

    Ptr<FeatureDetector> detector = FeatureDetector::create( "SURF" );

    if(detector.empty()) {
        PyErr_SetString(PyExc_ValueError, "Can not create detector or descriptor extractor or descriptor matcher of given types");
        return -1;
    }

    start_x = 0;
    end_x = img.size().width;

    detector->detect( img, keypoints );
    for (vector<KeyPoint>::iterator i = keypoints.begin(); i != keypoints.end(); i++)
    {
        if (i->pt.x > start_x && i->pt.x < end_x)
        {
            section_index = (int)ceil(i->pt.y / 10);
            section_num[section_index] = section_num[section_index] + 1;
            total = total + 1;
        }
    }
    avg = total / section_num.size();

    //检测特征点分布是否均匀
    int slice_total = 10 ;
    int slice_num = section_num.size() / slice_total;
    int slice_counter = 0;
    for (int m = 0; m < slice_total; m++)
    {
        for (int n = m * slice_num; n < (m+1) * slice_num; n++)
        {
            if ( section_num[n] >= avg )
            {
                slice_counter++;
                break;
            }
        }
    }
    if (slice_counter >= slice_total) {
        return -1;
    }

    //检测特征点主要分布区域[找最开始连续大于avg的Y]
    for (map<int,int>::iterator i = section_num.begin(); i != section_num.end(); i++) {
        if (i->second >= avg && flag == 0) {
            counter++;
        } else {
            counter = 0;
        }
        if (counter >= con_num && flag == 0) {
            Y = i->first;
            flag = 1;
        }
    }
    if (Y > con_num && Y < img.size().height / 4) {
        return (Y - con_num - 11) * slice_total < 0 ? 0 : (Y - con_num - 11) * slice_total; //fix
    } else if (Y > con_num){
        return (Y - con_num) * slice_total;
    }
    return Y * 10;
}

static PyObject *
tclip_tclip(PyObject *self, PyObject *args, PyObject *keywds)
{
    char *source_path = NULL;
    char *dest_path = NULL;
    int source_len, dest_len;
    int dest_height, dest_width;
    int result = 0;
    Mat image;
    Mat dest_image;
    Size tmp_size;
    float ratio_width = 0;
    float ratio_height = 0;
    float ratio = 0;
    int clip_top = 0;
    int clip_bottom = 0;
    int clip_left = 0;
    int clip_right = 0;

    static char *kwlist[] = {"source", "destination", "width", "height", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "ssii", kwlist,
                &source_path, &dest_path, &dest_width, &dest_height))
        return NULL;

    image = imread( source_path );
    if( !image.data ){
        PyErr_Format(PyExc_ValueError, "Fail to load image from '%s'", source_path);
        Py_RETURN_FALSE;
    }

    if (image.size().width * 3 <= image.size().height)
    {
        ratio = (float)dest_width / image.size().width;
        tmp_size = Size((int)(image.size().width * ratio), (int)(image.size().height * ratio));
        dest_image = Mat(tmp_size, CV_32S);
        resize(image, dest_image, tmp_size);
        clip_top = 0;
        clip_bottom = dest_height - dest_image.size().height;
        clip_left = 0;
        clip_right = 0;
        dest_image.adjustROI(clip_top, clip_bottom, clip_left, clip_right); //Mat& Mat::adjustROI(int dtop, int dbottom, int dleft, int dright)
        imwrite(dest_path, dest_image);
        Py_RETURN_TRUE;
    }

    ratio = (float)300.0 / image.size().width;
    tmp_size = Size((int)(image.size().width * ratio), (int)(image.size().height * ratio));
    dest_image = Mat(tmp_size, CV_32S);
    resize(image, dest_image, tmp_size);

    result = detectFace( dest_image );

    if (result == -1)
    {
        result = detectCharacter( dest_image );
    }

    result = result == -1 ? -1 : (int)((float)result / ratio);

    ratio_width = (float)dest_width / image.size().width;
    ratio_height = (float)dest_height / image.size().height;
    if (ratio_width > ratio_height) {
        ratio = ratio_width;
    } else {
        ratio = ratio_height;
    }

    result = result == -1 ? -1 : (int)((float)result * ratio);

    tmp_size = Size((int)(image.size().width * ratio), (int)(image.size().height * ratio));
    dest_image = Mat(tmp_size, CV_32S);
    resize(image, dest_image, tmp_size);

    //原图片 宽度小于高度
    if (ratio_width > ratio_height) {
        if (result == -1) {
            clip_top = -((dest_image.size().height - dest_height) / 2);
            clip_bottom = clip_top;
        } else {
            if (dest_image.size().height - result >= dest_height) {
                clip_top = -result;
                clip_bottom = -(dest_image.size().height - result - dest_height);
            } else {
                clip_top = -(dest_image.size().height - dest_height);
            }
        }
    } else {
        clip_left = -((dest_image.size().width - dest_width) / 2);
        clip_right = clip_left;
    }

    dest_image.adjustROI(clip_top, clip_bottom, clip_left, clip_right); //Mat& Mat::adjustROI(int dtop, int dbottom, int dleft, int dright)

    imwrite(dest_path, dest_image);

    Py_RETURN_NONE;
}

static PyMethodDef tclipMethods[] = {
    {"tclip", (PyCFunction)tclip_tclip, METH_VARARGS | METH_KEYWORDS, "tclip."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
inittclip(void)
{
    string face_config_path = "/usr/local/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml";
    if (!face_cascade.load(face_config_path)) {
        PyErr_Format(PyExc_ValueError, "Can not load classifier file！%s", face_config_path.c_str());
        return;
    }

    Py_InitModule3("tclip", tclipMethods,
            "tclip module that creates an extension type.");
}

