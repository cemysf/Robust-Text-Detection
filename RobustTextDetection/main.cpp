//
//  main.cpp
//  RobustTextDetection
//
//  Created by Saburo Okita on 05/06/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include <iostream>
#include <bitset>
#include <iterator> 
#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <unistd.h>
#include <boost/program_options.hpp>

#include "RobustTextDetection.h"
#include "ConnectedComponent.h"

using namespace std;
using namespace cv;
namespace po=boost::program_options;

int main(int argc, const char * argv[])
{
    string filename_input, foldername_temp, lang, filename_param;

    po::options_description desc("Options");
    desc.add_options()
            //("long_name,short_name", variable_assign , "message")
            ("help,h", "Help message")
            ("input,i", po::value<string>(&filename_input), "Image filename (required)")
            ("temp_out,t", po::value<string>(&foldername_temp)->default_value("/tmp"), "Path to create temp image files")
            ("lang,l", po::value<string>(&lang)->default_value("eng"),"Language for tesseract")
            ("param,p", po::value<string>(&filename_param)->default_value("params.txt"),"Parameter filename")
    ;
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(!vm.count("input") || (vm.count("help")))
    {
        cout << desc << endl;
        return 1;
    }

    namedWindow( "" );
    moveWindow("", 0, 0);
    
    Mat image = imread( filename_input );
    
    /* Quite a handful or params */
    RobustTextParam param;
    param.readFromFile(filename_param);
//    param.minMSERArea        = 10;
//    param.maxMSERArea        = 2000;
//    param.cannyThresh1       = 20;
//    param.cannyThresh2       = 100;
    
//    param.maxConnCompCount   = 10000;   //3000
//    param.minConnCompArea    = 75;
//    param.maxConnCompArea    = 600;
    
//    param.minEccentricity    = 0.1;
//    param.maxEccentricity    = 0.995;
//    param.minSolidity        = 0.4;
//    param.maxStdDevMeanRatio = 0.5;
    
    /* Apply Robust Text Detection */
    /* ... remove this temp output path if you don't want it to write temp image files */
    RobustTextDetection detector(param, foldername_temp );
    pair<Mat, Rect> result = detector.apply( image );
    
    /* Get the region where the candidate text is */
    Mat stroke_width( result.second.height, result.second.width, CV_8UC1, Scalar(0) );
    Mat(result.first, result.second).copyTo( stroke_width);
    
    
    /* Use Tesseract to try to decipher our image */
    tesseract::TessBaseAPI tesseract_api;
    tesseract_api.Init(NULL, lang.c_str());
    tesseract_api.SetImage((uchar*) stroke_width.data, stroke_width.cols, stroke_width.rows, 1, stroke_width.cols);
    
    string out = string(tesseract_api.GetUTF8Text());

    /* Split the string by whitespace */
    vector<string> splitted;
    istringstream iss( out );
    copy( istream_iterator<string>(iss), istream_iterator<string>(), back_inserter( splitted ) );
    
    /* And draw them on screen */
    /// TODO: try to make cvFontQt work (build opencv with qt support?)
    for(unsigned int i=0; i<splitted.size(); i++)
    {
    	cout << splitted[i] << endl;
    }
//    CvFont font = cvFontQt("Helvetica", 24.0, CV_RGB(0, 0, 0) );
//    Point coord = Point( result.second.br().x + 10, result.second.tl().y );
//    for( string& line: splitted ) {
//        addText( image, line, coord, font );
//        coord.y += 25;
//    }
    
    rectangle( image, result.second, Scalar(0, 0, 255), 2);
    
    /* Append the original and stroke width images together */
    cvtColor( stroke_width, stroke_width, CV_GRAY2BGR );
    Mat appended( image.rows, image.cols + stroke_width.cols, CV_8UC3 );
    image.copyTo( Mat(appended, Rect(0, 0, image.cols, image.rows)) );
    stroke_width.copyTo( Mat(appended, Rect(image.cols, 0, stroke_width.cols, stroke_width.rows)) );
    
    imshow("", appended );
    waitKey();
    
    return 0;
}
