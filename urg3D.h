#pragma once
#include <vector>
#include "URG.h"
#include "rs405cb.h"

class urg3D
{
private:
	static const int SERVO_OFFSET = -5;		// �T�[�{�̃I�t�Z�b�g(��t���p�x�����Ȃ���ݒ�)
	static const int MAX_NUM = 100000;		// (1081�_ * 20��)/�b - �����_��5�b���x�̃f�[�^��ۑ��\�ȗ̈�

	CURG urg;								// URG�̃N���X
	int num;
	pos_inten upos_inten[MAX_NUM];
	HANDLE hComm;
	HANDLE mutex;
	int tilt_low, tilt_high;
	float tilt_period;
	int terminate;
	static DWORD WINAPI ThreadFunc(LPVOID lpParameter);	// �X���b�h�̃G���g���[�|�C���g
	DWORD WINAPI ExecThread();				// �ʃX���b�h�œ��삷��֐�
	int Update();							// ����I(50ms)�ɌĂяo�����֐�

public:
	urg3D();								// �R���X�g���N�^
	virtual ~urg3D();						// �f�X�g���N�^
	int Init();								// �����ݒ�
	int Close();							// �I������
	int GetAllData(pos_inten *p, int max_no);		// URG�̃f�J���g���W�n�ł̏�Q���f�[�^���擾
	int GetSelectedData(int low, int high, pos *p, int max_no);
											// �������w�肵��urg�̃f�J���g���W�n�ł̏�Q���f�[�^���擾
	int Get2SelectedData(int low1, int high1, pos *p1, int *no1, int max_no1,
		int low2, int high2, pos *p2, int *no2, int max_no2);
											// �Q�̍������w�肵��urg�̃f�J���g���W�n�ł̏�Q���f�[�^���擾
	int Get3SelectedData(int low1, int high1, pos *p1, int *no1, int max_no1,
		int low2, int high2, pos *p2, int *no2, int max_no2,
		int low3, int high3, pos_inten *p3, int *no3, int max_no3, int min_intensity);
											// �R�̍������w�肵��urg�̃f�J���g���W�n�ł̏�Q���f�[�^���擾
	int SetTiltAngle(int low, int high, float period);
											// �`���g�A���O���̓����̐ݒ�
	int ClearData();						// �f�[�^���N���A����
};
