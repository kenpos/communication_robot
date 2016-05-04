#include <iostream>

#include<nishigulab_opencv248.h>

#include<stdio.h>
#include<stdlib.h>

#include <Windows.h>

#define img_width 640
#define img_height 480

/*************************************************************************
*グローバル変数
**************************************************************************/
//カスケードファイル
std::string face_cascade_name = "haarcascade_frontalface_alt.xml";
std::string eyes_cascade_name = "haarcascade_eye_tree_eyeglasses.xml";

/**************************************************************************
*main
***************************************************************************/
int main(int argc, char* argv[])
{
	///////////////////////////////////////////////////////
	//シリアル通信で開くポートの選択
	//////////////////////////////////////////////////////
	HANDLE hSerial = CreateFile("COM4",GENERIC_READ | GENERIC_WRITE,
                      0,
                      0,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL,
                      0);

	if (hSerial !=INVALID_HANDLE_VALUE)
    {
        printf("Port open \n");

        DCB dcbSerialParams;
        GetCommState(hSerial,&dcbSerialParams);

        dcbSerialParams.BaudRate = CBR_9600;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.Parity = NOPARITY;
        dcbSerialParams.StopBits = ONESTOPBIT;

        SetCommState(hSerial, &dcbSerialParams);
	}
	else
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			printf("Serial port doesn't exist! \n");
		}

		printf("Error while setting up serial port! \n");
	}

	char outputChars[] = "c";
	DWORD btsIO;

	/////////////////////////////////
	//Arduinoを操作する値
	/////////////////////////////////
	int servoPosition = 90;
	int servoOrientation = 0;

	///////////////////////////////////////////////////////
	//OpenCV関係
	//////////////////////////////////////////////////////

	// cameraサイズの設定
	CvSize size640x480 = cvSize(img_width, img_height);			

	 CvCapture* capture;
	 cv::Mat frame;					

	IplImage* p_imgOriginal;			// pointer to an image structure, this will be the input image from webcam
	IplImage* p_imgProcessed;			// pointer to an image structure, this will be the processed image
	IplImage* p_imgHSV;                 // pointer to an image structure, this will hold the image after the color has been changed from RGB to HSV
										// IPL is short for Intel Image Processing Library, this is the structure used in OpenCV 1.x to work with images

	CvMemStorage* p_strStorage;			// necessary storage variable to pass into cvHoughCircles()

	CvSeq* p_seqCircles;				// pointer to an OpenCV sequence, will be returned by cvHough Circles() and will contain all circles
										// call cvGetSeqElem(p_seqCircles, i) will return a 3 element array of the ith circle (see next variable)
	
	float* p_fltXYRadius;				// pointer to a 3 element array of floats
										// [0] => x position of detected object
										// [1] => y position of detected object
										// [2] => radius of detected object

	int i;								// loop counter
	char charCheckForEscKey;			// char for checking key press (Esc exits program)

	capture = cvCaptureFromCAM(0);	// 0 => use 1st webcam, may have to change to a different number if you have multiple cameras

	if(capture == NULL) {			// if capture was not successful . . .
		printf("error: capture is NULL \n");	// error message to standard out . . .
		getchar();								// getchar() to pause for user see message . . .
		return(-1);								// exit program
	}

											            // declare 2 windows
	cvNamedWindow("Original", CV_WINDOW_AUTOSIZE);		// original image from webcam
	cvNamedWindow("Processed", CV_WINDOW_AUTOSIZE);		// the processed image we will use for detecting circles

	p_imgProcessed = cvCreateImage(size640x480,			// 640 x 480 pixels (CvSize struct from earlier)
								   IPL_DEPTH_8U,		// 8-bit color depth
								   1);					// 1 channel (grayscale), if this was a color image, use 3

	p_imgHSV = cvCreateImage(size640x480, IPL_DEPTH_8U, 3); 



	// メインループ
	while(1) {								
		p_imgOriginal = cvQueryFrame(capture);		// get frame from webcam
		
		if(p_imgOriginal == NULL) {					// if frame was not captured successfully . . .
			printf("error: frame is NULL \n");		// error message to std out
			getchar();
			break;
		}

		//色相モデルをRGB形式（BGR）からHSVに変換
		//色のベースをHUE（色相）にすることで色を選びやすくするため
		cvCvtColor(p_imgOriginal, p_imgHSV, CV_BGR2HSV);

		//検出する色の範囲を設定
		cvInRangeS(p_imgHSV,				
				   cvScalar(75,  181, 256),			// 最低値 
				   cvScalar(75, 181, 256),			// 最高値 
				   p_imgProcessed);				

		p_strStorage = cvCreateMemStorage(0);	//cvHoughCircles()に飛んで必要最低限のメモリを割り当て 

		//処理された画像を滑らかにする
		//これにより次の処理で行う円を検出する
		cvSmooth(p_imgProcessed,		// 入力
				 p_imgProcessed,		// 出力
				 CV_GAUSSIAN,			// フィルタ選択【ガウシアンフィルタ】 (近くのピクセルに比重をおいて平均を取っていく)
				 9,						// フィルタのwidth
				 9);					// フィルタの height

		
		p_seqCircles = cvHoughCircles(p_imgProcessed,		        
									  p_strStorage,			        
									  CV_HOUGH_GRADIENT,	        
									  2,					        
									  p_imgProcessed->height / 4,	
									  100,						    
									  50,						    
									  10,	    					
									  400);		    				

		// サークルを見つけれていない場合
		if (p_seqCircles->total == 0)
		{
			// 初期化
			//最初にモータを180度の位置まで動かしてそこから物体が検出されるまで-5度ずつ動かしていく 
			//検出された位置に合わせてモータを5度ずつ動かして調整している
			

			if (servoOrientation == 0)
			{
				if (servoPosition >= 90)
					servoOrientation = 1;
				else
					servoOrientation = -1;
			}

			if (servoOrientation == 1)
			{
				outputChars[0] = 'l';
				WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

				// この部分はArduinoと同じコードを使ってる
				servoPosition+=5;

				if (servoPosition > 180)
				{
					servoPosition = 180;
					servoOrientation = -1;
				}
			}
			else
			{
				outputChars[0] = 'r';
				WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

				// この部分はArduinoと同じコードを使ってる
				servoPosition-=5;

				if (servoPosition < 0)
				{
					servoPosition = 0;
					servoOrientation = 1;
				}
			}
		}

		// 物体を検出した時の処理
		for(i=0; i < p_seqCircles->total; i++) {		

			p_fltXYRadius = (float*)cvGetSeqElem(p_seqCircles, i);	// from the sequential structure, read the ith value into a pointer to a float

			printf("ball position x = %f, y = %f, r = %f \n", p_fltXYRadius[0],		// x position of center point of circle
															  p_fltXYRadius[1],		// y position of center point of circle
															  p_fltXYRadius[2]);	// radius of circle

			// Reset servo orientation as the camera now has focus of a circle
			// Servo orientation is important only when the camera doesn't see a circle
			servoOrientation = 0;

			//横移動
			//　サークルが右端で見つかれば左にモータを回す
			//  ゆとりは100ピクセル
			if (p_fltXYRadius[0] > img_width-100)
			{
				outputChars[0] = 'l';
				WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

				servoPosition+=5;

				if (servoPosition > 180)
					servoPosition = 180;
			}

			// サークルが左端で見つかれば右にモータを回す
			if (p_fltXYRadius[0] < 100)
			{
				outputChars[0] = 'r';
				WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

				servoPosition-=5;

				if (servoPosition < 0)
					servoPosition = 0;
			}


			//縦移動
			if (p_fltXYRadius[1] > img_height-100)
			{
				outputChars[0] = 'd';
				WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

				servoPosition+=5;

				if (servoPosition > 180)
					servoPosition = 180;
			}

			// サークルが左端で見つかれば右にモータを回す
			if (p_fltXYRadius[1] < 100)
			{
				outputChars[0] = 'u';
				WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

				servoPosition-=5;

				if (servoPosition < 0)
					servoPosition = 0;
			}

			//物体の中心描画
			cvCircle(p_imgOriginal,										
					 cvPoint(cvRound(p_fltXYRadius[0]), cvRound(p_fltXYRadius[1])),		
					 3,													
					 CV_RGB(0,255,0),									
					 CV_FILLED);										
			
			//物体を囲む赤い円
			cvCircle(p_imgOriginal,										// どの画像に円を書くか
					 cvPoint(cvRound(p_fltXYRadius[0]), cvRound(p_fltXYRadius[1])),		// 円の中心
					 cvRound(p_fltXYRadius[2]),							// 円の半径
					 CV_RGB(255,0,0),									// 赤色描画
					 3);												// 円の太さ
		}	// end for

		//画像の表示
		cvShowImage("Original", p_imgOriginal);			
		cvShowImage("Processed", p_imgProcessed);		

		//メモリ解放
		cvReleaseMemStorage(&p_strStorage);				

		//10msのディレイ　モータとか処理被ると動かないのでそれの回避用
		charCheckForEscKey = cvWaitKey(10);				
		
		// ESCキーで終了
		if(charCheckForEscKey == 27) break;				

	}	// end while

	//cameraのメモリ解放
	cvReleaseCapture(&capture);					

	cvDestroyWindow("Original");
	cvDestroyWindow("Processed");

	// シリアルポート閉じる
    CloseHandle(hSerial);

	return(0);
}

