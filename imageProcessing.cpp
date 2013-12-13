#include "StdAfx.h"
#include "imageProcessing.h"

/*!
 * @class imageProcessing
 * @brief �摜�����ɂ��T���Ώێ҂����o���āC���̗L���ƕ�����m�点��N���X
 * @author Y.Hayashibara
 */

/*!
 * @brief �R���X�g���N�^
 */
imageProcessing::imageProcessing(void):
	terminate(0), capture(NULL)
{
}

/*!
 * @brief �f�X�g���N�^
 */
imageProcessing::~imageProcessing(void)
{
}

/*!
 * @brief ������
 * �J������摜�����̏��������s���C�摜�����̃X���b�h�𗧂��グ��D
 */
void imageProcessing::init(void)
{
	mutex = CreateMutex(NULL, FALSE, _T("IMAGE_PROCESSING_RESULT"));

	capture = cvCreateCameraCapture(0);
	if (capture == NULL) AfxMessageBox("Not Find Camera Device");

	// ���x����̃X���b�h���J�n
	DWORD threadId;	
	HANDLE hThread = CreateThread(NULL, 0, ThreadFunc, (LPVOID)this, 0, &threadId); 
	// �X���b�h�̗D�揇�ʂ��グ��
	SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);
}

/*!
 * @brief �I������
 */
void imageProcessing::close(void)
{
	terminate = 1;				// ���x����X���b�h�̒�~
	CloseHandle(mutex);
	cvDestroyWindow ("Camera");
}

/*!
 * @brief �����I�ɍs������
 * �摜�������s���C���ʂ��N���X�ϐ��ɑ������D
 * �摜�����������ʂ��E�B���h�E�ɕ\������D
 */
void imageProcessing::update(void)
{
	std::vector<cv::Rect> res;
	img = cvQueryFrame(capture);

#ifdef _DEBUG
	{
		static int prev_sec = 0;
		char s[100];
		time_t timer = time(NULL);
		struct tm *date = localtime(&timer);
		if ((date->tm_sec != prev_sec)&&(!img.empty())){	// �ۑ��͂P�b�ԂɂP���ȓ�
			prev_sec = date->tm_sec;
			sprintf(s, "image%04d%02d%02d%02d%02d%02d.bmp", date->tm_year+1900, date->tm_mon+1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);
			cv::imwrite(s, img);
		}
	}
#endif

	// ���ʂ̕`��
	namedWindow("Camera", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
	imshow( "Camera", img ); 
	waitKey(1000);														// 1000ms(1�b)�E�G�C�g
}

/*!
 * @brief ���o�������ʂ�n���ϐ�
 *
 * @param[out] cf    �m�M�x(0.0:�^�[�Q�b�g�ł͂Ȃ��`1.0:�^�[�Q�b�g�ł���)
 *
 * @return true:����, false:������
 */
bool imageProcessing::checkTarget(float *cf)
{
	// �Ăяo����ĉ摜�����C�������͊��ɏ����������ʂ�߂��D
	*cf = 0.0;			// �^�[�Q�b�g�ł͂Ȃ��D

	return false;
}

/*!
 * @brief �X���b�h�̃G���g���[�|�C���g
 *
 * @param[in] lpParameter �C���X�^���X�̃|�C���^
 * 
 * @return S_OK
 */
DWORD WINAPI imageProcessing::ThreadFunc(LPVOID lpParameter) 
{
	return ((imageProcessing*)lpParameter)->ExecThread();
}

/*!
 * @brief �ʃX���b�h�œ��삷��֐�
 * ���{�b�g�̐����ʃX���b�h�ōs���D
 *
 * @return S_OK
 */
DWORD WINAPI imageProcessing::ExecThread()
{
	while(!terminate){
		update();
	}
	return S_OK; 
}
