/*!
 * @file  estimatePos.h
 * @brief �����e�J�������Ȉʒu����v���O����
 * @date 2013.10.31
 * @author Y.Hayashibara
 */

#pragma once
#include "dataType.h"

float maxPI(float rad);									// �p�x��-PI�`PI�ɕϊ����邽�߂̊֐�

class estimatePos
{
public:
	estimatePos();										// �R���X�g���N�^
	virtual ~estimatePos();								// �f�X�g���N�^

private:
	// �ʒu�̕␳�ɗp����f�[�^�͈̔́i���{�b�g�����_�Ƃ��郏�[���h���W�j
	static const int search_x0 = -14000, search_x1 = 14000;		// �O������̒T���͈�(mm)
	static const int search_y0 = -14000, search_y1 = 14000;		// ���E�����̒T���͈�(mm)
	static const int dot_per_mm = 100;							// ��̃s�N�Z���̋���(mm)*/

	// ���t�@�����X�f�[�^
	static const int MAX_REF_DATA = 10000;
	int ref_data_no, is_ref_data_full;
	pos refData[MAX_REF_DATA];
	float odoX, odoY, odoThe;							// �^����ꂽ�ʒu(m, rad)
	float estX, estY, estThe, estVar;					// �v�Z���ċ��߂��ʒu(m, rad, ���U)
	float coincidence;

	// �������s���Ɍv�������f�[�^
	static const int MAX_DATA = 10000;
	int data_no;
	pos data[MAX_DATA];
	
	static const int MAX_PARTICLE = 500;
	struct particle_T particle[MAX_PARTICLE];
	float gaussian();									// �K�E�X���z���闐���𔭐�
	int evaluate();										// �p�[�e�B�N���̕]���ƃ\�[�g
	static int comp(const void *c1, const void *c2);	// �\�[�g�̂��߂̔�r�֐�
	float getVariance(float *ave_x, float *ave_y, float *ave_the);
														// ���U���v�Z����

public:
	int Init(float x, float y, float the);				// ������
	int Close();										// �I������

	int setOdometory(float x, float y, float the);		// �I�h���g���f�[�^�̓���
	int setDeltaPosition(float dx, float dy, float dthe);
														// ���肵���ړ��ʂ̓���
	int clearRefData();									// ���t�@�����X�f�[�^�̃N���A
	int addRefData(pos *p, int num);					// ���t�@�����X�f�[�^�̒ǉ�
	int setData(pos *p, int num);						// �v���f�[�^��ݒ肷��
	int getEstimatedPosition(float *x, float *y, float *the, float *var, float *coin);
														// ����ʒu�̎擾
	int calcualte();									// �p�[�e�B�N���t�B���^���g�������Ȉʒu����̏���
	int getParticle(struct particle_T *particle, int *num, int max_num);
														// �p�[�e�B�N���̎擾
	int getReferenceArea(int *x_min, int *y_min, int *x_max, int *y_max, int *dot_per_mm);
														// �Q�ƃG���A�̎擾
};

/* �g�����@�i�P�b�����x�̎����j�@���Ԃ�������̂ŁC�D��x�̒Ⴂ�X���b�h�Ŏ��s
 * 1) Init(x,y,the)�Ō��݂̈ʒu���w��
 * 2) addRefData(p, num)�ŎQ�ƃf�[�^�����
 * 3) setData(p, num)�Ōv���f�[�^�����
 * 4) setDeltaPosition(dx, dy, dthe)�ňړ���(���肵�������Ɗp�x)�����
 * 5) setOdometory(x, y, the)�ŃI�h���g�������
 * 6) calculate()�Ŏ��Ȉʒu�𐄒�
 * 7) getEstimatedPosition(&ex, &ey, &ethe, &var, &coincidence)�Ő��肵���ʒu���擾
 * 8) var��������coin���K�؂Ȓ��Ɏ��܂��Ă���ꍇ�́C���肵�����Ȉʒu��ύX
 * 9) 2)�ɖ߂�
 */
