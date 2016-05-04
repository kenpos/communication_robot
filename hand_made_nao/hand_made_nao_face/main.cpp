#include <iostream>

#include<nishigulab_opencv248.h>

#include<stdio.h>
#include<stdlib.h>

#include <Windows.h>

#define img_width 640
#define img_height 480

/*************************************************************************
*�O���[�o���ϐ�
**************************************************************************/
//�J�X�P�[�h�t�@�C��
std::string face_cascade_name = "haarcascade_frontalface_alt.xml";
std::string eyes_cascade_name = "haarcascade_eye_tree_eyeglasses.xml";

/**************************************************************************
*main
***************************************************************************/
int main(int argc, char* argv[])
{
	///////////////////////////////////////////////////////
	//�V���A���ʐM�ŊJ���|�[�g�̑I��
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
	//Arduino�𑀍삷��l
	/////////////////////////////////
	int servoPosition = 90;
	int servoOrientation = 0;

	///////////////////////////////////////////////////////
	//OpenCV�֌W
	//////////////////////////////////////////////////////

	// camera�T�C�Y�̐ݒ�
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



	// ���C�����[�v
	while(1) {								
		p_imgOriginal = cvQueryFrame(capture);		// get frame from webcam
		
		if(p_imgOriginal == NULL) {					// if frame was not captured successfully . . .
			printf("error: frame is NULL \n");		// error message to std out
			getchar();
			break;
		}

		//�F�����f����RGB�`���iBGR�j����HSV�ɕϊ�
		//�F�̃x�[�X��HUE�i�F���j�ɂ��邱�ƂŐF��I�т₷�����邽��
		cvCvtColor(p_imgOriginal, p_imgHSV, CV_BGR2HSV);

		//���o����F�͈̔͂�ݒ�
		cvInRangeS(p_imgHSV,				
				   cvScalar(75,  181, 256),			// �Œ�l 
				   cvScalar(75, 181, 256),			// �ō��l 
				   p_imgProcessed);				

		p_strStorage = cvCreateMemStorage(0);	//cvHoughCircles()�ɔ��ŕK�v�Œ���̃����������蓖�� 

		//�������ꂽ�摜�����炩�ɂ���
		//����ɂ�莟�̏����ōs���~�����o����
		cvSmooth(p_imgProcessed,		// ����
				 p_imgProcessed,		// �o��
				 CV_GAUSSIAN,			// �t�B���^�I���y�K�E�V�A���t�B���^�z (�߂��̃s�N�Z���ɔ�d�������ĕ��ς�����Ă���)
				 9,						// �t�B���^��width
				 9);					// �t�B���^�� height

		
		p_seqCircles = cvHoughCircles(p_imgProcessed,		        
									  p_strStorage,			        
									  CV_HOUGH_GRADIENT,	        
									  2,					        
									  p_imgProcessed->height / 4,	
									  100,						    
									  50,						    
									  10,	    					
									  400);		    				

		// �T�[�N����������Ă��Ȃ��ꍇ
		if (p_seqCircles->total == 0)
		{
			// ������
			//�ŏ��Ƀ��[�^��180�x�̈ʒu�܂œ������Ă������畨�̂����o�����܂�-5�x���������Ă��� 
			//���o���ꂽ�ʒu�ɍ��킹�ă��[�^��5�x���������Ē������Ă���
			

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

				// ���̕�����Arduino�Ɠ����R�[�h���g���Ă�
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

				// ���̕�����Arduino�Ɠ����R�[�h���g���Ă�
				servoPosition-=5;

				if (servoPosition < 0)
				{
					servoPosition = 0;
					servoOrientation = 1;
				}
			}
		}

		// ���̂����o�������̏���
		for(i=0; i < p_seqCircles->total; i++) {		

			p_fltXYRadius = (float*)cvGetSeqElem(p_seqCircles, i);	// from the sequential structure, read the ith value into a pointer to a float

			printf("ball position x = %f, y = %f, r = %f \n", p_fltXYRadius[0],		// x position of center point of circle
															  p_fltXYRadius[1],		// y position of center point of circle
															  p_fltXYRadius[2]);	// radius of circle

			// Reset servo orientation as the camera now has focus of a circle
			// Servo orientation is important only when the camera doesn't see a circle
			servoOrientation = 0;

			//���ړ�
			//�@�T�[�N�����E�[�Ō�����΍��Ƀ��[�^����
			//  ��Ƃ��100�s�N�Z��
			if (p_fltXYRadius[0] > img_width-100)
			{
				outputChars[0] = 'l';
				WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

				servoPosition+=5;

				if (servoPosition > 180)
					servoPosition = 180;
			}

			// �T�[�N�������[�Ō�����ΉE�Ƀ��[�^����
			if (p_fltXYRadius[0] < 100)
			{
				outputChars[0] = 'r';
				WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

				servoPosition-=5;

				if (servoPosition < 0)
					servoPosition = 0;
			}


			//�c�ړ�
			if (p_fltXYRadius[1] > img_height-100)
			{
				outputChars[0] = 'd';
				WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

				servoPosition+=5;

				if (servoPosition > 180)
					servoPosition = 180;
			}

			// �T�[�N�������[�Ō�����ΉE�Ƀ��[�^����
			if (p_fltXYRadius[1] < 100)
			{
				outputChars[0] = 'u';
				WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

				servoPosition-=5;

				if (servoPosition < 0)
					servoPosition = 0;
			}

			//���̂̒��S�`��
			cvCircle(p_imgOriginal,										
					 cvPoint(cvRound(p_fltXYRadius[0]), cvRound(p_fltXYRadius[1])),		
					 3,													
					 CV_RGB(0,255,0),									
					 CV_FILLED);										
			
			//���̂��͂ސԂ��~
			cvCircle(p_imgOriginal,										// �ǂ̉摜�ɉ~��������
					 cvPoint(cvRound(p_fltXYRadius[0]), cvRound(p_fltXYRadius[1])),		// �~�̒��S
					 cvRound(p_fltXYRadius[2]),							// �~�̔��a
					 CV_RGB(255,0,0),									// �ԐF�`��
					 3);												// �~�̑���
		}	// end for

		//�摜�̕\��
		cvShowImage("Original", p_imgOriginal);			
		cvShowImage("Processed", p_imgProcessed);		

		//���������
		cvReleaseMemStorage(&p_strStorage);				

		//10ms�̃f�B���C�@���[�^�Ƃ��������Ɠ����Ȃ��̂ł���̉��p
		charCheckForEscKey = cvWaitKey(10);				
		
		// ESC�L�[�ŏI��
		if(charCheckForEscKey == 27) break;				

	}	// end while

	//camera�̃��������
	cvReleaseCapture(&capture);					

	cvDestroyWindow("Original");
	cvDestroyWindow("Processed");

	// �V���A���|�[�g����
    CloseHandle(hSerial);

	return(0);
}

