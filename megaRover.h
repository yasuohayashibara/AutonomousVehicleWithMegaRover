#pragma once

#include <windows.h>

#define MEGA_ROVER_1_1

#define UP_BUTTON		0x10
#define DOWN_BUTTON		0x40
#define RIGHT_BUTTON	0x20
#define LEFT_BUTTON		0x80

class megaRover
{
private:
	static const int TREAD = 280;				//! �g���b�h�i�z�C�[���̋����j(mm)
	const float MAX_SPEED;						//! �ő呬�x(m/s)

	int is_speed_control_mode;					//! ���x���䃂�[�h�i1:���x���䃂�[�h�C0:���̑��j
	float refSpeedRight, refSpeedLeft;			//! ���E�z�C�[���̖ڕW���x(m/s)
	float speedRight, speedLeft;				//! ���E�z�C�[���̑��x(m/s)
	float odoX, odoY, odoThe;					//! �I�h���g���̈ʒu(m)�C�p��(rad)
	int deltaR, deltaL;							//! ���E�z�C�[���̉����x(m/s^2)
	int terminate;								//! �X���b�h�̔j���i1:�j��, 0:�p���j
	static DWORD WINAPI ThreadFunc(LPVOID lpParameter);	// �X���b�h�̃G���g���[�|�C���g
	DWORD WINAPI ExecThread();					// �ʃX���b�h�œ��삷��֐�
	int Update();								// �����I�ɍs������
	HANDLE mutex, comMutex;						// COM�|�[�g�̔r������
	int getEncoder(unsigned int *right, unsigned int *left);
												// �G���R�[�_�̒l�̎擾
public:
	megaRover();								// �R���X�g���N�^
	~megaRover();								// �f�X�g���N�^

	int init();									// ������
	int close();								// �I������
	int servoOn(int gain);						// �T�[�{�I��(gain:100)
	int setDelta(int right, int left);			// �ω��ʂ��w��
	int setMotor(int right, int left);			// ���[�^�ւ̃g���N����(-127-127)

	// speed control mode
	int setSpeedControlMode(int is_on);			// ���x���䃂�[�h�ւ̐؂�ւ�(1:���x����C0:�I�t)
	int setSpeed(float front, float rotate);	// �z�C�[���̖ڕW���x(m/s)
	int setArcSpeed(float front, float radius);	// ���{�b�g�̖ڕW���x(�O��C��])
	int getOdometory(float *x, float *y, float *the, int is_clear);	// �I�h���g���̎擾(m, rad) 1:�N���A
	int getJoyStick(float *x, float *y, int *b);
												// �W���C�X�e�B�b�N���̎擾
	int getReferenceSpeed(float *right, float *left);
	int setOdometoryAngle(float angle);			// �W���C���I�h���g���̂��߂ɕ��ʂ�ݒ肷��(rad)
	int getSpeed(float *rightSpeed, float *leftSpeed);
	int megaRover::clearOdometory();			// �I�h���g���̒l���N���A����D
};
