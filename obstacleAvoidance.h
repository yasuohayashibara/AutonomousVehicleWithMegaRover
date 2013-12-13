#pragma once
#include "dataType.h"

class obstacleAvoidance
{
public:
	obstacleAvoidance();
	virtual ~obstacleAvoidance();

private:
	static const int MARGIN = 200;				//! ��Q���Ƃ��Č��o����̈�̃��{�b�g�̕�����Ƃ��鍶�E�̃}�[�W�� (mm)
	static const int TREAD  = 282;				//! �g���b�h�i���E�̃^�C���̕��j (mm)
	static const int STOP_LENGTH = 500;			//! ��~���鋗�� (mm)
	static const int SLOW_DOWN_LENGTH = 1000;	//! �������J�n���鋗�� (mm)
	static const int REROUTE_PERIOD = 10;		//! ��Q�������o���Ă��烊���[�g����܂ł̎��� (s)

	// ��Q���̃f�[�^���󂯎��f�[�^�̈�
	static const int MAX_OBS_NUM = 1000;		//! ��Q���̍ő�l
	pos obs[MAX_OBS_NUM];						//! ��Q���̈ʒu�f�[�^
	int obs_num;								//! ��Q���̌�

	int is_obstacle;							//! ��Q���̗L���iCENTER�j
	int is_need_stop;
	float slow_down_factor;						//! �����̔䗦�iSTOP_LENGTH�ȉ���0, SLOW_DOWN_LENGTH��1�j
												//! �����ڕW���x�ɂ����邱�ƂŁC���X�ɒ�~����
	long obstacle_detect_time;					//! ��Q�������o��������(ms)
	float obstacle_detect_period;				//! ��Q���̌��o���Ă��鎞��(s)
	int is_reroute;								//! �����[�g�����ǂ���
	int reroute_direction;						//! �����[�g�̕����@(���F+1, �E -1)

	int getDataNum(int x_min, int y_min, int x_max, int y_max, int *nearest_x);
													// (x_min,y_min)-(x_max,y_max)�Ɋ܂܂���Q���̌����v�Z
													// nearest_x�ɍł��߂�x���W��߂�
	int isDetectObstacle(int right_center_left, int *min_x);
													// �E�C�����C���ɏ�Q�������邩���v������
	static const int RIGHT = -1, CENTER = 0, LEFT = +1;	//! �E�C�����C���̒萔

public:

	int Init();										// ������
	int Close();									// �I������
	int isObstacle();								// ��Q�������邩�ǂ�����߂�
	float getSlowDownFactor();						// �����̔䗦��߂�
	int setData(pos *p, int num);					// ��Q���̈ʒu�f�[�^��ݒ�
	int isReroute();					// �����[�g�����ǂ���
	int getRerouteDirection();						// �����[�g�̕���
	int finishReroute();							// �����[�g���I������
	int Update();									// ��Q������̏������s���D(setData���ɌĂяo�����)
	int isNeedStop();								// ��~���K�v�ȏ󋵂��ǂ�����߂��@(1:��~���K�v�C0:�K�v���Ȃ��j
};

/*
 * ���菇
 * 1) ��Q�������o�iSLOW_DOWN_LENGTH���猸���J�n�CSTOP_LENGTH�Œ�~�j
 * 2) �b�����Ԃ��o�߂����烊���[�g�J�n(��Q�����o����REROUTE_PERIOD�o��)�@�yToDo: �������疢�����z
 * 3) ���E�̋󂫏󋵂����o���ċ󂢂Ă���΂�����Ƀ����[�g�i�E�D��j
 * 4) �󂢂Ă��������90���^�[��(forceControl) ���S�ɐ����D��
 * 5) ��Q�����������`�F�b�N�i�����90���߂��ċt�����Ƀ^�[���j
 * 6) ���[�g�ɃI�t�Z�b�g�𑫂��Ēʏ�̐�����s���D(getRerouteOffset)
 * 7) ��Q�������o���Ȃ��Ȃ����猳�̕�����90������
 * 8) �ʏ�ʂ葖�s
 *
 */
