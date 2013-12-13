/*!
 * @file  estimatePos.cpp
 * @brief �����e�J�������Ȉʒu����v���O����
 * @date 2013.10.31
 * @author Y.Hayashibara
 */

#include "stdafx.h"
#include <math.h>
#include "estimatePos.h"

#define	M_PI	3.14159f

/*!
 * @brief �p�x��-PI�`PI�ɕϊ����邽�߂̊֐�
 *
 * @param[in] rad �p�x(rad)
 *
 * @return -PI�`PI�ɕϊ������p�x
 */
float maxPI(float rad){
	while(rad >  M_PI) rad -= (2.0f * M_PI);
	while(rad < -M_PI) rad += (2.0f * M_PI);
	return rad;
}

/*!
 * @class estimatePos
 * @brief ���Ȉʒu������s�����߂̃N���X
 * @author Y.Hayashibara
 */

/*!
 * @brief �R���X�g���N�^
 */
estimatePos::estimatePos():
odoX(0), odoY(0), odoThe(0), ref_data_no(0), is_ref_data_full(0), data_no(0),
estX(0), estY(0), estThe(0), estVar(0), coincidence(0)
{
}


/*!
 * @brief �f�X�g���N�^
 */
estimatePos::~estimatePos()
{
}


/*!
 * @brief ������
 *
 * @param[in] x   ���Ȉʒu��x���W(m)�i���t�@�����X�̃��[���h���W�n�j
 * @param[in] y   ���Ȉʒu��y���W(m)�i���t�@�����X�̃��[���h���W�n�j
 * @param[in] the ���Ȉʒu�̊p�x(rad)�i���t�@�����X�̃��[���h���W�n�j
 *
 * @return 0
 */
int estimatePos::Init(float x, float y, float the)
{
	// �ϐ��̏�����
	odoX = x, odoY = y, odoThe = the;
	estX = x, estY = y, estThe = the;
	ref_data_no = is_ref_data_full = 0;
	data_no = 0;
	estVar = coincidence = 0;
	the = maxPI(the);
	
	// �p�[�e�B�N���̏�����
	for(int i = 0; i < MAX_PARTICLE; i ++){
		particle[i].x    = x  ;
		particle[i].y    = y  ;
		particle[i].the  = the;
		particle[i].eval = 0  ;
	}
	// �Q�ƃf�[�^�̃N���A
	memset(refData, 0, sizeof(pos) * MAX_REF_DATA);

	return 0;
}


/*!
 * @brief �I������
 *
 * @return 0
 */
int estimatePos::Close()
{
	return 0;
}


/*!
 * @brief �I�h���g���f�[�^�̓���
 * �v�Z������O�ɁC�K�����͂���D�i��Q���̈ʒu�f�[�^�̊�ƂȂ�j
 *
 * @param[in] x   �I�h���g����x���W(m)�i���݂̃��[���h���W�n�j
 * @param[in] y   �I�h���g����x���W(m)�i���݂̃��[���h���W�n�j
 * @param[in] the �I�h���g���̊p�x(rad)�i���݂̃��[���h���W�n�j
 *
 * @return 0
 */
int estimatePos::setOdometory(float x, float y, float the)
{
	odoX = x;
	odoY = y;
	odoThe = maxPI(the);

	return 0;
}


/*!
 * @brief ���肵���ړ��ʂ̓���
 *
 * @param[in] dx   ���肵���ړ��ʂ�x���W(m)�i���t�@�����X�̃��[���h���W�n�j
 * @param[in] dy   ���肵���ړ��ʂ�y���W(m)�i���t�@�����X�̃��[���h���W�n�j
 * @param[in] dthe ���肵���ړ��ʂ̊p�x(rad)�i���t�@�����X�̃��[���h���W�n�j
 *
 * @return 0
 */
int estimatePos::setDeltaPosition(float dx, float dy, float dthe)	// m, rad
{
	const float thre = 0.5f;					// ���̐���ɐ����c�銄��
	const float var_fb = 0.01f, var_ang = 0.005f;
	int num = (int)(MAX_PARTICLE * thre);		// �����c��̐�

	for(int i = MAX_PARTICLE - 1; i >= 0; i --){
		particle[i].x   = particle[i % num].x + dx + cos(particle[i % num].the) * gaussian() * var_fb;
		particle[i].y   = particle[i % num].y + dy + sin(particle[i % num].the) * gaussian() * var_fb;
		particle[i].the = maxPI(particle[i % num].the + dthe + gaussian() * var_ang);
	}

	return 0;
}


/*!
 * @brief �p�[�e�B�N���t�B���^���g�������Ȉʒu����̏���
 *
 * @return 0
 */
int estimatePos::calcualte()
{
	evaluate();										// �p�[�e�B�N���̕]��
	estVar = getVariance(&estX, &estY, &estThe);	// ���t�@�����X�̃��[���h���W�n�ɂ����鐄��ʒu�����߂�D
	
	return 0;
}


/*!
 * @brief ���t�@�����X�f�[�^�̃N���A
 * ���̊֐����Ăяo�����܂ŁC���t�@�����X�̋����f�[�^�͒~�ς��ꑱ����D
 *
 * @return 0
 */
int estimatePos::clearRefData()
{	
	ref_data_no = is_ref_data_full = 0;

	return 0;
}


/*!
 * @brief ���t�@�����X�f�[�^�̒ǉ�
 *
 * @param[in] p ���t�@�����X�ƂȂ��Q���̈ʒu�f�[�^�i���t�@�����X�̃��[���h���W�n�j
 * @param[in] num ���t�@�����X�f�[�^�̌�
 *
 * @return 0
 */
int estimatePos::addRefData(pos *p, int num)
{
	for(int i = 0; i < num; i ++){
		refData[ref_data_no ++] = p[i];
		if (ref_data_no >= MAX_REF_DATA){
			is_ref_data_full = 1;
			ref_data_no = 0;
		}
	}
	
	return 0;
}


/*!
 * @brief �v���f�[�^��ݒ肷��
 *
 * @param[in] p �v��������Q���̈ʒu�f�[�^�i���݂̃��[���h���W�n�j
 * @param[in] num �f�[�^�̌�
 *
 * @return 0
 */
int estimatePos::setData(pos *p, int num)
{
	data_no = min(num, MAX_DATA);
	for(int i = 0; i < data_no; i ++){
		data[i] = p[i];
	}
	return 0;
}


/*!
 * @brief ����ʒu�̎擾
 *
 * @param[in] x    ���肵���ʒu��x���W(m)�i���t�@�����X�̃��[���h���W�n�j
 * @param[in] y    ���肵���ʒu��y���W(m)�i���t�@�����X�̃��[���h���W�n�j
 * @param[in] the  ���肵���ʒu�̊p�x(rad)�i���t�@�����X�̃��[���h���W�n�j
 * @param[in] var  ���U(0.0-1.0)
 * @param[in] coin ��v�x(0.0-1.0)
 *
 * @return 0
 */
int estimatePos::getEstimatedPosition(float *x, float *y, float *the, float *var, float *coin)
{
	*x    = estX  ;
	*y    = estY  ;
	*the  = estThe;
	*var  = estVar;
	*coin = coincidence;

	return 0;
}


/*!
 * @brief �K�E�X���z���闐���𔭐�
 *
 * @return �K�E�X���z���闐��
 */
float estimatePos::gaussian()
{
	float x, y, s, t;

	do{
		x = (float)rand()/RAND_MAX;
	} while (x == 0.0);											// x��0�ɂȂ�̂������
	y = (float)rand()/RAND_MAX;
	s = sqrt(-2.0f * log(x));
	t = 2.0f * M_PI * y;

	return s * cos(t);											// �W�����K���z�ɏ]���[������
}


/*!
 * @brief �p�[�e�B�N���̕]���ƃ\�[�g
 *
 * @return 0
 */
int estimatePos::evaluate()
{
	static const int point_wide = 4;							// ���_��^����ׂ̐�
	static const int point[point_wide + 1] = {16,8,4,2,1};		// �^���链�_
	static const int wide = point_wide * 2 + 1;					// �e�[�u����x�����̃T�C�Y
	static char table[wide][wide];								// ���_�̃e�[�u��

	static const int num_x = (search_x1 - search_x0)/dot_per_mm;
	static const int num_y = (search_y1 - search_y0)/dot_per_mm;
	static const int min_x = search_x0 / dot_per_mm;
	static const int max_x = search_x1 / dot_per_mm;
	static const int min_y = search_y0 / dot_per_mm;
	static const int max_y = search_y1 / dot_per_mm;
	static char map[num_y][num_x];

	// �e�[�u���̍쐬
	for(int i = 0; i < wide; i ++){
		for(int j = 0; j < wide; j ++){
			table[i][j] = point[max(abs(i - point_wide), abs(j - point_wide))];
		}
	}

	// �}�b�v�̍쐬 (estX, estY)�𒆐S
	memset(map, 0, num_x*num_y);								// map�̃N���A
	int ref_no = ref_data_no;
	if (is_ref_data_full) ref_no = MAX_REF_DATA;				// �Q�Ƃ���f�[�^�̐�

	for(int i = 0;i < ref_no; i ++){
		int x = (int)((refData[i].x - estX * 1000) / dot_per_mm) - min_x;		// �}�b�v��̈ʒu���v�Z
		int y = (int)((refData[i].y - estY * 1000) / dot_per_mm) - min_y;		
		if ((x < point_wide)||(x >= (num_x - point_wide))||
			(y < point_wide)||(y >= (num_y - point_wide))) continue;
		for(int yp = 0; yp < wide; yp ++){
			for(int xp = 0; xp < wide; xp ++){
				int xt = x + (xp - point_wide);
				int yt = y + (yp - point_wide);
				map[yt][xt] = max(map[yt][xt], table[yp][xp]);	// ���]���̍������̂��̗p
			}			
		}
	}

	// �p�[�e�B�N���̕]��
	pos p;
	for(int i = 0;i < MAX_PARTICLE; i ++){
		float px   = particle[i].x * 1000;				// ���݂̃p�[�e�B�N���̈ʒu(m->mm)
		float py   = particle[i].y * 1000;
		float pthe = particle[i].the;
		particle[i].eval = 0;							// �]����0�ɖ߂�
		for(int j = 0; j < data_no; j ++){
			float dx0 = data[j].x - odoX * 1000;
			float dy0 = data[j].y - odoY * 1000;
			float dx  =   dx0 * cos(odoThe) + dy0 * sin(odoThe);		// ���݌v�����Ă��鋗���f�[�^�i���{�b�g���W�j
			float dy  = - dx0 * sin(odoThe) + dy0 * cos(odoThe);
			// �v���f�[�^���p�[�e�B�N���̈ʒu����ɕϊ�
			p.x = (int)(dx * cos(pthe) - dy * sin(pthe) + px);
			p.y = (int)(dx * sin(pthe) + dy * cos(pthe) + py);
			int xt = (int)((p.x - estX * 1000) / dot_per_mm) - min_x;	// �}�b�v��̈ʒu���v�Z
			int yt = (int)((p.y - estY * 1000) / dot_per_mm) - min_y;
			if ((xt >= 0)&&(xt < num_x)&&(yt >= 0)&&(yt < num_y)){
				particle[i].eval += map[yt][xt];
			}
		}
	}
	// �]���ɂ��\�[�e�B���O
	qsort(particle, MAX_PARTICLE, sizeof(struct particle_T), comp);
	
	// ��v�x���v�Z�@(0-1)
	coincidence = 0;
	for(int i = 0;i < MAX_PARTICLE; i ++){
		coincidence += particle[i].eval;
	}
	if (data_no){
		coincidence /= (MAX_PARTICLE * point[0] * data_no);
	} else {
		coincidence = 0;
	}
	return 0;
}

/*!
 * @brief �\�[�g�̂��߂̔�r�֐�
 *
 * @param[in] c1 �p�[�e�B�N���P�̃|�C���^
 * @param[in] c2 �p�[�e�B�N���Q�̃|�C���^
 *
 * @return 1:c1�̕]���������C0:�]���������C-1:c2�̕]��������
 */
int estimatePos::comp(const void *c1, const void *c2)
{
	const struct particle_T *p1 = (struct particle_T *)c1;
	const struct particle_T *p2 = (struct particle_T *)c2;

	if      (p2->eval > p1->eval) return 1;
	else if (p2->eval < p1->eval) return -1;
	else return 0;
}

/*!
 * @brief ���U���v�Z����
 *
 * @param[out] ave_x   x�̕��ϒl(m)
 * @param[out] ave_y   y�̕��ϒl(m)
 * @param[out] ave_the �p�x�̕��ϒl(rad)
 *
 * @return ���U�̒l
 */
float estimatePos::getVariance(float *ave_x, float *ave_y, float *ave_the)
{
	float sum_x = 0, sum_y = 0, sum_t = 0, sumv = 0;
	float ax, ay, at;

	for(int i = 0; i < MAX_PARTICLE; i ++){
		sum_x  += particle[i].x;
		sum_y  += particle[i].y;
		sum_t  += maxPI(particle[i].the - particle[0].the);
		// -PI��PI�ŕ��ς��āC0�ɂȂ邱�Ƃ�h���Dthe��-PI�`PI�ł��邱�Ƃ��O��
	}
	ax = sum_x / MAX_PARTICLE;
	ay = sum_y / MAX_PARTICLE;
	at = maxPI(sum_t / MAX_PARTICLE + particle[0].the);

	for(int i = 0; i < MAX_PARTICLE; i ++){
		float dx = particle[i].x   - ax;
		float dy = particle[i].y   - ay;
		float dt = maxPI(particle[i].the - at);
		sumv += dx * dx + dy * dy + at * at;	// m��rad�����݂����Ă��ǂ����H
	}
	*ave_x = ax, *ave_y = ay, *ave_the = at;

	return sumv / MAX_PARTICLE;
}

/*!
 * @brief �p�[�e�B�N���̎擾
 *
 * @param[out] p       �p�[�e�B�N���f�[�^�̃|�C���^
 * @param[out] num     �擾�����p�[�e�B���N�̐�
 * @param[in]  max_num �ő�̃p�[�e�B�N����
 *
 * @return 0
 */
int estimatePos::getParticle(struct particle_T *p, int *num, int max_num)
{
	*num = min(max_num, MAX_PARTICLE);
	struct particle_T *q = particle;
	for(int i = 0; i < *num; i ++){
		*p ++ = *q ++;
	}

	return 0;
}

/*!
 * @brief �Q�ƃG���A�̎擾
 *
 * @param[out] x_min x���̍ŏ��l(mm)
 * @param[out] y_min y���̍ŏ��l(mm)
 * @param[out] x_max x���̍ő�l(mm)
 * @param[out] y_max y���̍ő�l(mm)
 * @param[out] dot_per_mm ��̃s�N�Z���̋���(mm)
 *
 * @return 0
 */
int estimatePos::getReferenceArea(int *x_min, int *y_min, int *x_max, int *y_max, int *dot_per_mm)
{
	*x_min = search_x0;
	*y_min = search_y0;
	*x_max = search_x1;
	*y_max = search_y1;
	*dot_per_mm = this->dot_per_mm;

	return 0;
}
