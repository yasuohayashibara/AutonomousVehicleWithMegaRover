#pragma once
#include "comm.h"

#define	IMU_COM_PORT	4

class imu
{
public:
	imu();										// �R���X�g���N�^
	virtual ~imu();								// �f�X�g���N�^

public:
	CComm comm;									// �ʐM�|�[�g�̃N���X

	int Init(int com_port);						// ������
	int Close();								// �I������
	int Reset();								// IMU�̃��Z�b�g
	int GetAngleStart();						// �p�x�̎擾�̊J�n
	int GetAngle(float *x, float *y, float *z);	// �p�x�̎擾
};
