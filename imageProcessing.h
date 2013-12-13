#pragma once

#include <opencv2\\opencv.hpp>

using namespace cv;

class imageProcessing
{
private:
	CvCapture *capture;				//! �r�f�I�L���v�`���\����
	Mat img;						//! �摜
	
	int terminate;					//! �X���b�h�̔j���i1:�j��, 0:�p���j
	static DWORD WINAPI ThreadFunc(LPVOID lpParameter);	// �X���b�h�̃G���g���[�|�C���g
	DWORD WINAPI ExecThread();		// �ʃX���b�h�œ��삷��֐�
	HANDLE mutex, comMutex;			// COM�|�[�g�̔r������
public:
	imageProcessing(void);			// �R���X�g���N�^
	~imageProcessing(void);			// �f�X�g���N�^
	void init(void);				// ������
	void close(void);				// �I������
	void update(void);				// �����I�ɍs������
	bool checkTarget(float *cf);	// ���o�������ʂ�n���ϐ�
};
