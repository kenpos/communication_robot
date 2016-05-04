#include <iostream>
//#include <opencv2/opencv.hpp>

#include "nishigulab_opencv248.h"

//using namespace cv;

#include <Windows.h>


#define img_width 640
#define img_height 480

HANDLE initialize_serial();

void release_serial(HANDLE hSerial);
void servo_move(int face_x,int face_y );
void serch_face();
void face_tracking(cv::VideoCapture cap ,std::vector<cv::Rect> faces ,cv::CascadeClassifier cascade);
void colorExtraction(cv::Mat* src, cv::Mat* dst,
    int code,
    int ch1Lower, int ch1Upper,
    int ch2Lower, int ch2Upper,
    int ch3Lower, int ch3Upper
    );

	/////////////////////////////////
	//Arduino�𑀍삷��l
	/////////////////////////////////
	int servoPosition = 90;
	int servoPosition1 = 90;

	int servoOrientation = 0;
	HANDLE hSerial;
	char outputChars[] = "c";
	DWORD btsIO;



	/*******************************************************
	*
	*main�֐�
	*
	*********************************************************/
int main(int, char**)
{
	//�|�[�g�J��+Servo�̏�����
	initialize_serial() ;

    cv::VideoCapture cap(0); // �f�t�H���g�J�������I�[�v��
    if(!cap.isOpened())  // �����������ǂ������`�F�b�N
        return -1;
    cv::namedWindow("window1",1);
	 cv::namedWindow("raw",1);
	  cv::namedWindow("maskedImage",1);
  // ���ފ�̓ǂݍ���
  std::string cascadeName = "haarcascade_frontalface_alt2.xml";
  cv::CascadeClassifier cascade;
  if(!cascade.load(cascadeName))
    return -1;
 
    std::vector<cv::Rect> faces;

   while(true)
    { cv::Mat frame;
        cv::Mat input_image;
        cv::Mat output_image;
        cap >> frame; // �J��������V�����t���[�����擾
                cvtColor(frame, input_image, CV_BGR2GRAY);//�猟�o�̓O���[��ok
                cv::equalizeHist( input_image, input_image);
                output_image=frame;     //output�̓J���[�ŁD

		//�璊�o
     //face_tracking(cap ,faces ,cascade);
	 
	colorExtraction(&frame, &output_image, CV_BGR2HSV, 150, 165, 100, 255, 70, 255);
	
        if(cv::waitKey(30) >= 0) break;

    }

    release_serial(hSerial);
    // VideoCapture �f�X�g���N�^�ɂ��C�J�����͎����I�ɏI����������܂�
    return 0;
}


HANDLE initialize_serial()
{
	
	hSerial = CreateFile("COM4",GENERIC_READ | GENERIC_WRITE,
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



	WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

	return hSerial;
}

void release_serial(HANDLE hSerial){
	// �V���A���|�[�g����
    CloseHandle(hSerial);
}

void serch_face(){
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

void servo_move(int face_x,int face_y ){

			// Reset servo orientation as the camera now has focus of a circle
			// Servo orientation is important only when the camera doesn't see a circle
			servoOrientation = 0;

			//���ړ�
			//�@�T�[�N�����E�[�Ō�����΍��Ƀ��[�^����
			//  ��Ƃ��100�s�N�Z��
			if (face_x > img_width-150)
			{
				outputChars[0] = 'r';
				WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

				servoPosition+=2;

				if (servoPosition > 180)
					servoPosition = 180;
			}

			// �T�[�N�������[�Ō�����ΉE�Ƀ��[�^����
			if (face_x < 150)
			{
				outputChars[0] = 'l';
				WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

				servoPosition-=2;

				if (servoPosition < 0)
					servoPosition = 0;
			}


			//�c�ړ�
			if (face_y > img_height-150)
			{
				outputChars[0] = 'u';
				WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

				servoPosition1+=2;

				if (servoPosition1 > 180)
					servoPosition1 = 180;
			}

			// �T�[�N�������[�Ō�����ΉE�Ƀ��[�^����
			if (face_y < 150)
			{
				outputChars[0] = 'd';
				WriteFile(hSerial, outputChars, strlen(outputChars), &btsIO, NULL);

				servoPosition1-=2;

				if (servoPosition1 < 0)
					servoPosition1 = 0;
			}
}

void face_tracking(cv::VideoCapture cap ,std::vector<cv::Rect> faces ,cv::CascadeClassifier cascade){
  cv::Mat frame;
        cv::Mat input_image;
        cv::Mat output_image;
        cap >> frame; // �J��������V�����t���[�����擾
                cvtColor(frame, input_image, CV_BGR2GRAY);//�猟�o�̓O���[��ok
                cv::equalizeHist( input_image, input_image);
                output_image=frame;     //output�̓J���[�ŁD
                
                //(�摜,�o�͒Z�`,�k���X�P�[��,�Œ�Z�`��,�t���O�H,�ŏ��Z�`)
                 cascade.detectMultiScale(input_image, faces,
                           1.3, 2,
                           CV_HAAR_SCALE_IMAGE
                           ,
                           cv::Size(50, 50));

                  // ���ʂ̕`��
                std::vector<cv::Rect>::const_iterator r = faces.begin();
                for(; r != faces.end(); ++r) {
                        cv::Point center;
                        int radius;
                        center.x = cv::saturate_cast<int>((r->x + r->width*0.5));
                        center.y = cv::saturate_cast<int>((r->y + r->height*0.5));
                        radius = cv::saturate_cast<int>((r->width + r->height)*0.25);
                        cv::circle( output_image, center, radius, cv::Scalar(80,80,255), 3, 8, 0 );
						
						int cx =center.x;
						int cy =center.y;
                        printf("faceID%d,x=%d,y=%d,width=%d,height=%d\n",r,r->x,r->y,r->width,r->height);

						//��̕����܂Ō����P���T�N�D�I�I
						servo_move(cx,cy);
  }
				//if(r == faces.end());
				//serch_face();
				

                        //���ɂ��邽�ߔ��]������
                    //flip(output_image,output_image,1);
                    imshow("window1", output_image);
}

void colorExtraction(cv::Mat* src, cv::Mat* dst,int code,
    int ch1Lower, int ch1Upper,
    int ch2Lower, int ch2Upper,
    int ch3Lower, int ch3Upper
    )
{
    cv::Mat colorImage;
	cv::Mat raw;
    int lower[3];
    int upper[3];

    cv::Mat lut = cv::Mat(256, 1, CV_8UC3);   
	
    cv::cvtColor(*src, colorImage, code);

    lower[0] = ch1Lower;
    lower[1] = ch2Lower;
    lower[2] = ch3Lower;

    upper[0] = ch1Upper;
    upper[1] = ch2Upper;
    upper[2] = ch3Upper;

    for (int i = 0; i < 256; i++){
        for (int k = 0; k < 3; k++){
            if (lower[k] <= upper[k]){
                if ((lower[k] <= i) && (i <= upper[k])){
                    lut.data[i*lut.step+k] = 255;
                }else{
                    lut.data[i*lut.step+k] = 0;
                }
            }else{
                if ((i <= upper[k]) || (lower[k] <= i)){
                    lut.data[i*lut.step+k] = 255;
                }else{
                    lut.data[i*lut.step+k] = 0;
                }
            }
        }
    }

    //LUT���g�p���ē�l��
    cv::LUT(colorImage, lut, colorImage);

    //Channel���ɕ���
    std::vector<cv::Mat> planes;
    cv::split(colorImage, planes);

    //�}�X�N���쐬
    cv::Mat maskImage;
    cv::bitwise_and(planes[0], planes[1], maskImage);
    cv::bitwise_and(maskImage, planes[2], maskImage);

    //�o��
    cv::Mat maskedImage;
    src->copyTo(maskedImage, maskImage);
    *dst = maskedImage;
	cv::imshow("raw", raw);
	cv::imshow("maskedImage", maskedImage);
	
}


