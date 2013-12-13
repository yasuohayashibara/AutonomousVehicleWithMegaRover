#pragma once

#include "estimatePos.h"

class navi
{
public:
	navi();							// �R���X�g���N�^
	virtual ~navi();				// �f�X�g���N�^

private:
	int is_record;					//! �ۑ����[�h���ǂ����̃t���O
	int is_play;					//! �Đ����[�h���ǂ����̃t���O
	int step;						//! waypoint�̔ԍ�
	float step_period;				//! ����waypoint�ɓ��B����܂ł̎���
	long time0;						//! �O��waypoint��ʉ߂�������(ms)

	// �t�@�C�����o�͊֘A
	char target_filename[256];		//! �f�[�^��ۑ�����t�@�C����
	long seek;						//! �f�[�^���Q�Ƃ��Ă���ꏊ
	long getSeek(int num);			// �w�肵��waypoint�̔ԍ��܂ŁC�t�@�C���̓ǂݍ��݈ʒu���ړ�����
	int saveNextOdometory(float x, float y, float the);	// �I�h���g���̕ۑ�
	int saveNextData(pos *p, int num);					// ��Q���̈ʒu�f�[�^�̕ۑ�
	int loadNextOdoAndData(float *x, float *y, float *the, pos *p, int *num, int max_num);
									// ����waypoint�Ə�Q���̋����f�[�^��ǂݍ���
	
	// target�֘A
	float tarX, tarY, tarThe;		// ����waypoint�̈ʒu(m,rad)
	int isPassTarget(float x, float y, float the);
									// waypoint�ɋ߂Â������ǂ������`�F�b�N����֐�

	// �v��������Q���̈ʒu�f�[�^�̈ꎞ�ۊǏꏊ
	static const int MAX_DATA = 10000;	//! ��Q���̈ʒu�f�[�^�̍ő��
	int data_no;						//! ��Q���̈ʒu�f�[�^�̌�
	pos data[MAX_DATA];					//! ���O���̃f�[�^

	// �Q�Ƃ����Q���̈ʒu�f�[�^�̈ꎞ�ۊǏꏊ
	static const int MAX_REF_DATA = 10000;	//! �Q�Ƃ����Q���̈ʒu�f�[�^�̍ő��
	int ref_data_no;						//! �Q�Ƃ����Q���̈ʒu�f�[�^�̌�
	pos refData[MAX_REF_DATA];				//! �Q�Ƃ����Q���̈ʒu�f�[�^

	// �I�h���g��
	float odoX0, odoY0, odoThe0;	//! ��O�̃E�F�C�|�C���g��ʉ߂������̃I�h���g��(m, rad)

	// ����l
	float estX0, estY0, estThe0;	//! ��O�̃E�F�C�|�C���g��ʉ߂������̈ʒu�̐���l(m,rad)
	float estX,  estY,  estThe ;	//! ���݂̈ʒu�̐���l

	static DWORD WINAPI ThreadFunc(LPVOID lpParameter);	// �v���̃X���b�h
	DWORD WINAPI ExecThread();
	DWORD threadId;					//! �X���b�h ID	
	HANDLE hThread;					//! �X���b�h�̃n���h��

	estimatePos est_pos;			//! ���Ȉʒu����̃N���X�̃C���X�^���X
	float coincidence;				//! ��v�x (0-1)
	int selfLocalization(float x, float y, float the, float dx, float dy, float dthe);
									// ���Ȉʒu����
	// �T���p�̏���
	int is_search_object;			//! �T���Ώێ҂̗L��
	int is_search_mode;				//! �T���Ώێ҂�T�����郂�[�h
	int search_mode;				//! �T���̃��[�h�i�V�[�P���X�̏����Ɏg�p����j
	float searchX, searchY;			//! �T������ʒu
	float retX, retY;				//! �T�����[�h����߂�ʒu�i�O���[�o�����W�j
	float forwardSpeed, rotateSpeed;//! ���x�ڕW(m/s)
	int isPassSearchObject(float x, float y, float the);	// �T���Ώە��ɋ߂Â������ǂ������`�F�b�N����֐�
	int searchProcess();									// �T���̃v���Z�X
	int stop();												// ��~����i�T�����[�h�̂݁j
	int turnToPos(float x, float y, float margin_angle);	// �ڕW(x,y)�Ɍ�������
	int moveToPos(float x, float y, float margin_distance);	// �ڕW(x,y)�ɋߊ��

	int is_reroute_mode;
	int reroute_direction;
	int reroute_mode;
	float rerouteX0, rerouteY0, rerouteThe0;				// �����[�g���n�߂�����x,y,the
	int rerouteProcess();
	int turn90deg(int direction, float the0, float margin_angle);
	int moveForward(float length, float x0, float y0);

	static const int RIGHT = -1, CENTER = 0, LEFT = +1;	//! �E�C�����C���̒萔
	int is_need_stop;

public:
	int Init();						// ������
	int Close();					// �I������
	int setOdometory(float x, float y, float the);	// �I�h���g����ݒ肷��
	int setStep(int num);			// waypoint�̔ԍ����Z�b�g����
	int getStep();					// waypoint�̔ԍ����擾����
	int setData(pos *p, int num);	// ��Q���̈ʒu�f�[�^�̐ݒ�
	int getEstimatedPosition(float *x, float *y, float *the);				// ���肵���ʒu�̎擾
	int getTargetPosition(float *x, float *y, float *the, float *period);	// waypoint�̎擾
	int getTargetArcSpeed(float *front, float *radius);						// waypoint�Ɍ��������{�b�g�̑��x�Ɖ�]���a�����߂�
	int getParticle(struct particle_T *particle, int *num, int max_num);	// �p�[�e�B�N���̃f�[�^���擾����
	int getCoincidence(float *coincidence);									// ��v�x���擾
	int setTargetFilename(char *filename = NULL);							// �f�[�^��ۑ�����t�@�C�������w�肷��
	int setRecordMode(int is_record);										// �ۑ����[�h�̐ݒ�
	int setPlayMode(int is_play);											// �Đ����[�h�̐ݒ�
	int isPlayMode();														// PlayMode���ǂ�����߂�
	int getRefData(pos *p, int *num, int max_num);							// �Q�ƃf�[�^�̎擾
	int setSearchPoint(pos p);												// �T���Ώۂ̌��_��ݒ�
	int isSearchMode();														// �T�����[�h���ǂ�����߂�(0:�T�����[�h�łȂ��C1:�T�����[�h�j
	int getSpeed(float *forward, float *rotate);							// ���ڃz�C�[���̑��x���i�߂���D(�T�����[�h�Ŏg�p)
	float distaceFromPreviousSearchPoint();									// �O��̒T���Ώۂ���̋�����߂�(m)
	int setRerouteMode(int direction);										// �����[�g���[�h�ɂ���D
	int isRerouteMode();
	int setNeedStop(int is_need_stop);
};

/*
 * �g����
 * 
 * ���ۑ�(record)
 * 1) setTargetFilename("temp.csv")�ȂǂŃt�@�C�������w��i���ԂȂǂ������I�ɓ����悤�ɂ������j
 * 2) setRecordMode(1)�ŕۑ����[�h�ɂ���D
 * 3) �f�[�^���擾���邲�Ƃ�
 *    setOdometory(x,y,the), setData(p,num)�Ńt�@�C����ۑ��i���Ƀ��[���h���W�j
 * 4) setRecordMode(0)�ŕۑ����[�h����������D�i�K�������K�v�Ȃ��j
 * 
 * �r������̕ۑ��ɂ͑Ή����Ă��Ȃ��i�v�����j
 *
 * ���Đ�(play)
 * 1) setTargetFilename("temp.csv")�ȂǂŃt�@�C�������w��i���ԂȂǂ������I�ɓ����悤�ɂ������j
 * 2) setPlayMode(1)�ōĐ����[�h�ɂ���D
 * 3) setOdometory(x,y,the)�Ō��݈ʒu����́i���[���h���W�n�j
 * 4) setData(p, num)�Ōv���f�[�^����́i���[���h���W�n�j
 * 5) getEstimatedPosition(&x,&y,&the)�Ō��݈ʒu���擾�i���[���h���W�n�j
 * 6) getTargetPosition(&x,&y,&the,&period)�ŖڕW�ʒu���擾�i���[���h���W�n�j
 * 7) getTargetArcSpeed(&front,&radius)���擾�C����ɏ]���z�C�[���𐧌�
 * 8) 3)�ɖ߂�
 * 9) setPlayMode(0)�ōĐ����[�h����������C�i���Ȃ������K�v�Ȃ��j
 *
 */

/*
 * ���Đ�(play)���̏����菇
 *
 * ���S�[���̃��C����ʉ߂��閈��
 * 1)�O�X��̃S�[�����C���ʉߎ��̐���ʒu���x�[�X�Ƃ��Č��݈ʒu���Z�o����
 * 2)���̃S�[���ɓ��B���邽�߂́C�ڕW���x�C���񔼌a�����߂�D
 *
 * 3) �O��̃I�h���g���́y����ʒu�z��setPosition(float x, float y, float the)�ɓ����
 * 4) �O�񂩂�ۑ������f�[�^���ŐV�̃I�h���g���y�ʒu�z�̑��΍��W�ɕϊ�
 * 5) estimatePos::setData�Ƀf�[�^����́i���{�b�g���W)
 * 6) �O�񂩂�̃I�h���g���̍�����estimatePos::setOdometory�ɓ��́i���[���h���W�j
 *    �y���Ӂz�ړ������͎ԗւ���C�����̓I�h���g���́y����ʒu�z����v�Z����
 * 7) estimatePos::calculate()�Ōv�Z
 * 8) estimatePos::getOdometory(&x,&y,&the,&var,&coincidence)�Ő���ʒu���擾
 * 9) var���������Ccoincidence���傫���ꍇ�́C���肵���I�h���g���̈ʒu���C��
 *    �����łȂ��ꍇ�́C�P���ɃI�h���g���́y����ʒu�z�ɍ����𑫂�
 *
 *    ��������Ȉʒu�Ƃ��āC�S�[���Ɍ��������߂̖ڕW�O���𐶐����Ď��̃S�[�����C���܂Ő��䂷��
 */

/*
 * �I�h���g���̎��
 * 1) record���̃I�h���g��
 * 2) play���̃I�h���g��
 * 3) play����record���̃I�h���g���ɍ��킹���I�h���g��
 */
