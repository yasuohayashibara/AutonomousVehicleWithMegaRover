/*!
 * @file  navi.cpp
 * @brief �i�r�Q�[�V�����̃N���X
 * @date 2013.10.31
 * @author Y.Hayashibara
 *
 * ���Ȉʒu����C�ڕW�o�H�̎擾�C���[�^�̐���𓝊�����i�r�Q�[�V�����̃N���X
 * �P�jInit()�ŏ��������s���D
 * �Q�jsetOdometory(x,y,the)�Ō��݂�
 * �R�j
 */

#include "stdafx.h"
#include <math.h>
#include <mmsystem.h>
#include <time.h>
#include "navi.h"
#include "logger.h"
#include "megaRover.h"

#define	M_PI	3.14159f

/*!
 * @class navi
 * @brief �i�r�Q�[�V�������s���N���X
 * @author Y.Hayashibara
 */

/*!
 * @brief �R���X�g���N�^
 */
navi::navi():
step(0), is_record(0), is_play(0), seek(0), step_period(1), time0(0),
tarX(0), tarY(0), tarThe(0), data_no(0), ref_data_no(0),
odoX0(0), odoY0(0), odoThe0(0),
estX0(0), estY0(0), estThe0(0), estX(0), estY(0), estThe(0),
hThread(NULL), coincidence(0),
is_search_object(0), is_search_mode(0), search_mode(0),
searchX(10000.0f), searchY(10000.0f),									// ���ɉ����ʒu������
is_reroute_mode(0), reroute_direction(RIGHT), reroute_mode(0),
forwardSpeed(0), rotateSpeed(0), is_need_stop(0)
{
}

/*!
 * @brief �f�X�g���N�^
 */
navi::~navi()
{
}

/*!
 * @brief ������
 * ���̃N���X���g�p����Ƃ��ɍŏ��ɂP��Ăяo���D
 * ���肵�����Ȉʒu��(0,0)�C�p�x0rad�Ƃ��Ă���D
 *
 * @return 0
 */
int navi::Init()
{
//	WaitForSingleObject(hThread, 3000);						// ���Ȉʒu�̐�����s���Ă���ꍇ�͑҂�
	step = 0;
	seek = 0;
	time0 = 0;
	tarX = tarY = tarThe = 0;
	ref_data_no = 0;
	odoX0 = odoY0 = odoThe0 = 0;
	data_no = 0;
	estX0 = estY0 = estThe0 = 0;
	estX = estY = estThe = 0;	
	est_pos.Init(0,0,0);									// ���Ȉʒu����̏�����

	return 0;
}

/*!
 * @brief �I������
 *
 * @return 0
 */
int navi::Close()
{
	est_pos.Close();										// ���Ȉʒu����̏I������

	return 0;
}

/*!
 * @brief �I�h���g����ݒ肷��D
 *
 * �I�h���g������͂����^�C�~���O�ŁC�ȉ��̏������s���D
 * [record]
 * �������i���݂P�b�j���ƂɈʒu���L�^����D�����play��waypoint�Ƃ��Ďg�p����D
 * [play]
 * waypoint�ɓ��B����(������x�߂Â�)���ƂɈȉ��̏������s���D
 * ����waypoint�ƎQ�Ƃ����Q���̈ʒu�f�[�^�����[�h���āC��������Ȉʒu����̊֐��ɓn���D
 * ����ɂ��C����waypoint�ɓ��B����܂łɁCwaypoint��ʉ߂������̎��Ȉʒu�𐄒肷��D
 * ���Ȉʒu����̊m�x�i�����f�[�^�̈�v�x�j���Ⴂ�ꍇ�́C�I�h���g���݂̂ő��s����D
 *
 * @param[in] x   �I�h���g����x���W(m)
 * @param[in] y   �I�h���g����y���W(m)
 * @param[in] the �I�h���g���̊p�x(rad) -PI�`PI
 *
 * @return 0:����I, 1:�S�[���ɓ���
 */
int navi::setOdometory(float x, float y, float the)
{
	if (is_record){
		long time = timeGetTime();									// step_period�ȏ�̊Ԋu���J���ĕۑ��i�E�F�C�|�C���g�ƂȂ�j
		if ((time - time0) > (int)(step_period * 1000)){
			saveNextOdometory(x, y, the);							// �I�h���g���̈ʒu��ۑ�
			step ++;												// �X�e�b�v���C���N�������g
			time0 = time;
		}
	}
	
	if (is_play){
		float dx0 = x - odoX0, dy0 = y - odoY0, dthe0 = the - odoThe0;	// �I�h���g���̍���
		float dthe = estThe0 - odoThe0;								// ���肵���p�x���g���ĕ␳
		float dx = dx0 * cos(dthe) - dy0 * sin(dthe);
		float dy = dx0 * sin(dthe) + dy0 * cos(dthe);
		if (is_reroute_mode){
			if (!rerouteProcess()) is_reroute_mode = 0;			
		} else if (is_search_mode){										// �T���Ώێ҂�T�����郂�[�h�̏ꍇ
			if (!searchProcess()) is_search_mode = 0;			
		} else if (isPassTarget(estX, estY, estThe)){				// waypoint���p�X�����ꍇ
			if (loadNextOdoAndData(&tarX, &tarY, &tarThe, refData, &ref_data_no, MAX_REF_DATA)){
				return 1;											// �S�[���ɓ���
			}
			step ++;												// waypoint�ԍ��̃C���N�������g
			if (WAIT_TIMEOUT != WaitForSingleObject(hThread, 0)){	// �X���b�h���I�����Ă���ꍇ
																	// ���̖ڕW�ʒu�C�f�[�^�̃��[�h
				selfLocalization(x, y, the, dx, dy, dthe0);			// ���Ȉʒu����̎擾�y�я����̊J�n
				odoX0 = x, odoY0 = y, odoThe0 = the;				// �E�F�C�|�C���g�ʉߎ��̃I�h���g����ۑ��i���ΓI�Ȓl���擾���邾���Ȃ̂ŁC����Ă��Ă����Ȃ��j
				estX0 += dx, estY0 += dy, estThe0 += dthe0;			// �E�F�C�|�C���g�ʉߎ��̈ʒu�̐���l�i����㏑�������j
				dx0 = dy0 = dthe0 = dx = dy = 0;					// ���̎��Ȉʒu�̌v�Z�̂��߂ɃN���A
			}
			data_no = 0;											// �f�[�^�̃N���A
		}
		if (is_search_object){
			if (isPassSearchObject(estX, estY, estThe)){			// �T���Ώۂ����ɗ�����
				is_search_mode = 1;									// �T�����[�h�Ɉڍs
				search_mode = 0;									// �T���̃V�[�P���X�����Ɏg�p����l���N���A����
				is_search_object = 0;								// �T���Ώۂ��N���A����
				retX = estX, retY = estY;							// �߂�ʒu��ۑ�
			}
		}
		// �ʒu�̐���l���v�Z
		estX   = dx    + estX0  ;
		estY   = dy    + estY0  ;
		estThe = dthe0 + estThe0;
	}

	return 0;
}


/*!
 * @brief ���Ȉʒu������s���D
 *
 * 1) ���Ȉʒu����̃X���b�h���I�����Ă��邩���`�F�b�N�D�I�����Ă��Ȃ����return
 * 2) ���肵�����Ȉʒu���擾�D���U�ƈ�v�x���K���Ȕ͈͂ł���΁C���̒l��waypoint�ʉߎ��̐���l�Ƃ���D
 * 3) estimatePos(���Ȉʒu����̃N���X)�ɁC�Q�Ƃ����Q���̈ʒu�f�[�^��n���D
 * 4) estimatePos�Ɍv��������Q���̈ʒu�f�[�^��n���D
 * 5) estimatePos�Ɉړ�����������n���D
 * 6) estimatePos�Ɍ��݂̃I�h���g���̒l��n���D
 * 7) ���Ȉʒu���v�Z����X���b�h�̊J�n
 *
 * @param[in] x    �I�h���g����x���W(m)
 * @param[in] y    �I�h���g����y���W(m)
 * @param[in] the  �I�h���g���̊p�x(rad) -PI�`PI
 * @param[in] dx   �ړ�����x���W�̒l(m)
 * @param[in] dy   �ړ�����y���W�̒l(m)
 * @param[in] dthe �ړ�������]�p�x(rad) -PI�`PI
 *
 * @return -1:���Ȉʒu����v�Z���̂��߁C�������s��Ȃ��D0:�������J�n
 */
int navi::selfLocalization(float x, float y, float the, float dx, float dy, float dthe)
{
	static float max_var = 100000.0f, min_coin = 0.1f;
	
	// �X���b�h���I�����Ă��Ȃ��ꍇ�́C����������-1��߂�
	if (WAIT_TIMEOUT == WaitForSingleObject(hThread, 0)) return -1;

	float ex, ey, ethe, var;
	est_pos.getEstimatedPosition(&ex, &ey, &ethe, &var, &coincidence);
	int lv = (int)(coincidence * 10);					// �m�x��10�i�K�ɂ��āC�����ŏo�͂���D
	if ((lv >= 0)&&(lv <= 10)){
		char fn[10];
		sprintf(fn, "%d.wav", lv);
		PlaySound(fn, NULL, SND_FILENAME | SND_ASYNC);
	}

	if ((var < max_var)&&(coincidence > min_coin)){		// �M����������l�̏ꍇ�͓���ւ�
		estX0 = ex, estY0 = ey, estThe0 = ethe;
	}
	// ���̖ڕW�ʒu�C�f�[�^�̃��[�h
	est_pos.addRefData(refData, ref_data_no);
	est_pos.setData(data, data_no);
	est_pos.setDeltaPosition(dx, dy, dthe);				// �I�h���g���̍����Ƃ��ē��͂���i����p�x�ŕ␳��j
	est_pos.setOdometory(x, y, the);					// �I�h���g���̈ʒu�𒼐ړ��͂���
	// ���Ȉʒu�̌v�Z�̃X���b�h���J�n
	hThread = CreateThread(NULL, 0, ThreadFunc, (LPVOID)this, 0, &threadId); 
	// �X���b�h�̗D�揇�ʂ�������
	SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
	
	return 0;
}

/*!
 * @brief waypoint�̔ԍ����Z�b�g����@�y���`�F�b�N�֐��z
 * �r������J�n���邽�߂Ɏ����������C���삷�邩�ǂ������`�F�b�N���Ă��Ȃ��D
 *
 * @param[in] num waypoint�̔ԍ�
 *
 * @return -1:play���[�h�ł͂Ȃ��D0:����I��
 */
int navi::setStep(int num)
{
	if (!is_play) return -1;
	step = num;
	seek = getSeek(num);
	
	return 0;
}

/*!
 * @brief waypoint�̔ԍ����擾����
 *
 * @return waypoint�̔ԍ�
 */
int navi::getStep()
{
	return step;
}

/*!
 * @brief ��Q���̈ʒu�f�[�^�̐ݒ�
 *
 * [record]
 * ��Q���̈ʒu�f�[�^�̕ۑ�
 * [play]
 * waypoint�Ԃ̏�Q���̈ʒu�f�[�^��ۑ����āC���Ȉʒu���肪�s����悤�ɏ�������
 * MAX_DATA(10000)�ȏ�̃f�[�^�͖��������D
 *
 * @param[in] p ��Q���̈ʒu�f�[�^�̃|�C���^
 * @param[in] num ��Q���̈ʒu�f�[�^�̐�
 *
 * @return ���ݕۑ����Ă���f�[�^�̐�(play�̏ꍇ��0)
 */
int navi::setData(pos *p, int num)
{
	if (is_record) saveNextData(p, num);
	if (is_play){								// MAX_DATA�܂Ńf�[�^��ǉ����Ă���
		num = min(num, MAX_DATA - data_no);
		for(int i = 0; i < num; i ++){
			data[data_no + i] = p[i];
		}
		data_no += num;
	}

	return data_no;
}

/*!
 * @brief ���肵���ʒu�̎擾
 *
 * @param[out] x   ���肵��x���W�̒l(m)
 * @param[out] y   ���肵��y���W�̒l(m)
 * @param[out] the ���肵����]�p�x(rad)
 *
 * @return 0
 */
int navi::getEstimatedPosition(float *x, float *y, float *the)
{
	*x   = estX  ;
	*y   = estY  ;
	*the = estThe;

	return 0;
}

/*!
 * @brief �Q�ƃf�[�^�̎擾
 * �\���y�ь��؂̂��߂ɁC���ݎQ�Ƃ��Ă����Q���̈ʒu�f�[�^��߂�
 *
 * @param[out] p �Q�Ƃ��Ă����Q���̈ʒu�f�[�^
 * @param[out] num �ʒu�f�[�^�̐�
 *
 * @return 0
 */
int navi::getRefData(pos *p, int *num, int max_num)
{
	*num = min(max_num, MAX_REF_DATA);
	pos *q = refData;
	for(int i = 0; i < *num; i ++){
		*p ++ = *q ++;
	}

	return 0;
}

/*!
 * @brief waypoint�̎擾
 *
 * @param[out] x waypoint��x���W�̒l(m)
 * @param[out] y waypoint��y���W�̒l(m)
 * @param[out] the waypoint�̊p�x(rad)
 * @param[out] period ���B�܂ł̎���(sec) �y���g�p�z
 *
 * @return 0
 */
int navi::getTargetPosition(float *x, float *y, float *the, float *period)
{
	*x = tarX;
	*y = tarY;
	*the = tarThe;
	*period = 0;		// �v�ύX

	return 0;
}


/*!
 * @brief �f�[�^��ۑ�����t�@�C�������w�肷��
 *
 * @param[in] filename �t�@�C����(NULL�̏ꍇ�����I�Ƀt�@�C���������������)
 *
 * @return 0
 */
int navi::setTargetFilename(char *filename)
{
	if (filename != NULL){
		strcpy(target_filename, filename);
	} else {
		char s[100];
		time_t timer = time(NULL);
		struct tm *date = localtime(&timer);
		sprintf(s, "navi%04d%02d%02d%02d%02d.csv", date->tm_year+1900, date->tm_mon+1, date->tm_mday, date->tm_hour, date->tm_min);
		strcpy(target_filename, s);
	}
	return 0;
}

/*!
 * @brief �I�h���g���̕ۑ�
 *
 * @param[in] x �I�h���g����x���W(m)
 * @param[in] y �I�h���g����y���W(m)
 * @param[in] the �I�h���g���̊p�x(rad) -PI�`PI
 *
 * @return 
 */
int navi::saveNextOdometory(float x, float y, float the)
{
	FILE *fp;

	fp = fopen(target_filename, "a");
	fprintf(fp, "o, %d, %d, %d\n", (int)(x * 1000), (int)(y * 1000), (int)(the * 180 / M_PI));
	fclose(fp);

	return 0;
}

/*!
 * @brief ��Q���̈ʒu�f�[�^�̕ۑ�
 *
 * @param[in] p ��Q���̈ʒu�f�[�^�̃|�C���^
 * @param[in] num ��Q���̐�
 *
 * @return 0
 */
int navi::saveNextData(pos *p, int num)
{
	FILE *fp;

	fp = fopen(target_filename, "a");
	for(int i = 0; i < num; i ++){
		fprintf(fp, "u, %d, %d, %d\n", p[i].x, p[i].y, p[i].z);
	}
	fclose(fp);

	return 0;
}

/*!
 * @brief ����waypoint�Ə�Q���̋����f�[�^��ǂݍ���
 *
 * @param[out] x waypoint��x���W(m)
 * @param[out] y waypoint��y���W(m)
 * @param[out] the waypoint�̊p�x(rad) -PI�`PI
 * @param[out] p ��Q���̈ʒu�f�[�^
 * @param[out] num ��Q���̐�
 * @param[out] max_num �擾�����Q���̍ő�l (num�͂���ȏ�̐��ɂȂ�Ȃ�)
 *
 * @return 0:���̖ڕW�ʒu������ꍇ�C-1:���̖ڕW�ʒu�������ꍇ�i�S�[���ɓ����j
 */
int navi::loadNextOdoAndData(float *x, float *y, float *the, pos *p, int *num, int max_num)
{
	FILE *fp;
	long seek0;
	char c;
	int d0 = 0, d1 = 0, d2 = 0;
	int n = 0, is_first_odo = 1;
	int ret = -1;

	fp = fopen(target_filename, "r");
	fseek(fp, seek, SEEK_SET);
	while(true){
		seek0 = ftell(fp);
		if (EOF == fscanf(fp, "%c, %d, %d, %d", &c, &d0, &d1, &d2)) break;
		ret = 0;
		if (c == 'o'){
			if (is_first_odo == 0) break;			// �Q�ڂ̃I�h���g���ŏI��
			*x   = (float)d0 / 1000.0f;
			*y   = (float)d1 / 1000.0f;
			*the = (float)d2 * M_PI / 180.0f; 
			is_first_odo = 0;
		} else if (c == 'u'){
			if (n < max_num){
				p[n].x = d0;
				p[n].y = d1;
				p[n].z = d2;
				n ++;
			}
		}
	}
	fclose(fp);

	seek = seek0;									// �I�h���g���̒��O�̔Ԓn��ۑ�
	*num = n;

	return ret;
}

/*!
 * @brief �w�肵��waypoint�̔ԍ��܂ŁC�t�@�C���̓ǂݍ��݈ʒu���ړ�����@�y�����؁z
 *
 * @param[in] num waypoint�̔ԍ�
 *
 * @return �t�@�C���̓ǂݍ��݈ʒu
 */
long navi::getSeek(int num)
{
	FILE *fp;
	long seek0;
	char s[100];
	int d0, d1, d2;
	int n = 0;

	if (num == 0) return 0;

	fp = fopen(target_filename, "r");
	while(true){
		seek0 = ftell(fp);
		fscanf(fp, "%s, %d, %d, %d\n", s, &d0, &d1, &d2);
		if (!strcmp(s, "o")){
			n ++;
			if (n >= num) break;
		}
	}
	fclose(fp);

	return seek0;
}

/*!
 * @brief �ۑ����[�h�̐ݒ�
 * �ۑ����[�h��I�������ꍇ�C�Đ����[�h�́C���������D
 *
 * @param[in] is_recode �ۑ����[�h�ɂ��邩�̃t���O(1:�ۑ����[�h�C0:�ۑ����[�h������)
 *
 * @return 0
 */
int navi::setRecordMode(int is_record)
{
	if (is_record) is_play = 0;				// �ۑ��ƍĐ��̔r������
	this->is_record = is_record;
	
	return 0;
}

/*!
 * @brief �Đ����[�h�̐ݒ�
 * �Đ����[�h��I�������ꍇ�C�ۑ����[�h�́C���������D
 *
 * @param[in] is_play �Đ����[�h�ɂ��邩�̃t���O(1:�Đ����[�h�C0:�Đ����[�h������)
 *
 * @return 0
 */
int navi::setPlayMode(int is_play)
{
	if (is_play) is_record = 0;				// �ۑ��ƍĐ��̔r������
	this->is_play = is_play;

	return 0;
}

/*!
 * @brief �Đ����[�h���ǂ�����߂��D
 *
 * @return 0:play���[�h�ł͂Ȃ��D1:play���[�h
 */
int navi::isPlayMode()
{
	return is_play;
}

/*!
 * @brief waypoint�ɋ߂Â������ǂ������`�F�b�N����֐�
 *
 * @param[in] x ���{�b�g��x���W�̈ʒu(m)
 * @param[in] y ���{�b�g��y���W�̈ʒu(m)
 * @param[in] the ���{�b�g�̊p�x(rad)
 *
 * @return 0:�߂Â��Ă��Ȃ��C1:�߂Â��Ă���
 */
int navi::isPassTarget(float x, float y, float the)
{
	const float MARGIN = -0.5f;				// �ǂ̒��x�O�ɗ�����E�F�C�|�C���g�ʉ߂Ƃ݂Ȃ���
	float dx, dy, tx, ty;

	dx = x - tarX;							// target����̑��Έʒu�ɕϊ�
	dy = y - tarY;
				
	// �����target�̌����ɍ��킹�ĕϊ�
	tx =   dx * cos(tarThe) + dy * sin(tarThe);
	ty = - dx * sin(tarThe) + dy * cos(tarThe);

	return (tx >= MARGIN);					// ���{�b�g��x���W��0.3�𒴂��Ă���΃p�X
}

/*!
 * @brief �T���Ώە��ɋ߂Â������ǂ������`�F�b�N����֐�
 *
 * @param[in] x ���{�b�g��x���W�̈ʒu(m)
 * @param[in] y ���{�b�g��y���W�̈ʒu(m)
 * @param[in] the ���{�b�g�̊p�x(rad)
 *
 * @return 0:�߂Â��Ă��Ȃ��C1:�߂Â��Ă���
 */
int navi::isPassSearchObject(float x, float y, float the)
{
	const float MARGIN = -0.5f;				// �ǂ̒��x�O�ɗ�����E�F�C�|�C���g�ʉ߂Ƃ݂Ȃ���
	float dx, dy, tx, ty;

	dx = x - searchX;						// �T���Ώۂ���̑��Έʒu�ɕϊ�
	dy = y - searchY;
				
	// �����target�̌����ɍ��킹�ĕϊ�
	tx =   dx * cos(tarThe) + dy * sin(tarThe);
	ty = - dx * sin(tarThe) + dy * cos(tarThe);

	return (tx >= MARGIN);					// ���{�b�g��x���W��0.3�𒴂��Ă���΃p�X
}


/*!
 * @brief �X���b�h�̃G���g���[�|�C���g
 *
 * @param[in] lpParameter �C���X�^���X�̃|�C���^
 * 
 * @return S_OK
 */
DWORD WINAPI navi::ThreadFunc(LPVOID lpParameter) 
{
	return ((navi*)lpParameter)->ExecThread();
}

/*!
 * @brief �ʃX���b�h�œ��삷��֐�
 * �p�[�e�B�N���t�B���^�̌v�Z��ʃX���b�h�ōs���D
 *
 * @return S_OK
 */
DWORD WINAPI navi::ExecThread()
{
	for(int i = 0; i < 10;i ++){	// �R���s���[�^�̌v�Z�\�͂ɂ�蒲������
		est_pos.calcualte();
	}

	return S_OK;
}

/*!
 * @brief waypoint�Ɍ��������{�b�g�̑��x�Ɖ�]���a�����߂�
 *
 * @param[out] front �O������̑��x(m/s)
 * @param[out] radius ��]���a(m)
 *
 * @return 0
 */
int navi::getTargetArcSpeed(float *front, float *radius)
{
#ifdef MEGA_ROVER_1_1
	const float MAX_SPEED = 0.5f;
#else
	const float MAX_SPEED = 0.3f;
#endif

	float dx0, dy0, dx, dy;
	
	dx0 = tarX - estX;
	dy0 = tarY - estY;
	
	dx =   dx0 * cos(estThe) + dy0 * sin(estThe);
	dy = - dx0 * sin(estThe) + dy0 * cos(estThe);

	if (dy == 0){
		if (dx != 0){
			dy = dx * 0.001f;
		} else {
			*radius = *front = 0;
			return -1;
		}
	}

	*radius = (dx * dx + dy * dy) / (2.0f * dy);
	*front = min(fabs(*radius) * atan2(dx, fabs(*radius - dy)), MAX_SPEED);

	return 0;
}

/*!
 * @brief �p�[�e�B�N���̃f�[�^���擾����
 * �\���ƌ��ؗp�Ƀp�[�e�B�N���̃f�[�^���擾����֐�
 *
 * @param[out] particle �p�[�e�B�N���̃f�[�^�̃|�C���^
 * @param[out] num �p�[�e�B�N���̐�
 * @param[out] max_num �p�[�e�B�N���̃f�[�^�̃|�C���^
 *
 * @return 0
 */
int navi::getParticle(struct particle_T *particle, int *num, int max_num)
{
	static const int MAX_PARTICLE = 1000;
	static struct particle_T p[MAX_PARTICLE];
	
	est_pos.getParticle(p, num, MAX_PARTICLE);
	struct particle_T *q = p;
	for(int i = 0; i < *num; i ++){
		*particle ++ = *q ++;
	}

	return 0;
}

/*!
 * @brief ��v�x���擾
 *
 * @param[out] coincidence ��v�x
 *
 * @return 0
 */
int navi::getCoincidence(float *coincidence)
{
	*coincidence = this->coincidence;

	return 0;
}

/*!
 * @brief �T���Ώۂ̌��_��ݒ�
 * ���̊֐����Ăяo���Ǝb�����̓_�Ɍ������ă��{�b�g���ړ�����D
 *
 * @param[in] p �T���Ώۂ̌��_
 *
 * @return 0
 */
int navi::setSearchPoint(pos p)
{
	searchX = p.x / 1000.0f;	// �T������Ώە��̈ʒu
	searchY = p.y / 1000.0f;	// �T������Ώە��̈ʒu
	is_search_object = 1;		// �T������Ώە��̗L��

	return 0;
}

/*!
 * @brief �T���̃v���Z�X
 *
 * @return 1:�p�����C0:�I��
 */
int navi::searchProcess()
{
	const float MARGIN_ANGLE = 0.1f;		// ���̃��[�h�Ɉڂ�p�x�덷(rad)
	const float MARGIN_DIST_HUMAN = 1.0f;	// �l�ɋ߂Â�����(m)
	const float MARGIN_DIST_RETURN = 0.1f;	// �o�H�ɖ߂�Ƃ��Ɏ��̃��[�h�Ɉڂ�ʒu�덷(m)

	enum SEQUENCE {
		INIT = 0,
		TURN_TARGET,
		MOVE_TARGET,
		INDICATE_FIND,
		INDICATE_FIND_WAIT,
		TURN_RETURN,
		MOVE_WAYPOINT,
		TURN_WAYPOINT,
		FINISH
	};
	
	int res = 1;

	static long time_mode0 = 0;				// �O��mode��؂�ւ������̎���(ms)
	static int search_mode0 = -1;			// �O���search_mode
	if (search_mode != search_mode0){
		search_mode0 = search_mode;
		time_mode0 = timeGetTime();
	}
	float mode_period = (timeGetTime() - time_mode0) / 1000.0f;

	LOG("search_mode:%d\n", search_mode);
	if (mode_period < 1.0f) stop();
	else {
		switch(search_mode){
			case INIT:{
				stop();					// ��~
				if (mode_period >= 1.0f) search_mode ++;
				break;
				   }
			case TURN_TARGET:{
				if (!turnToPos(searchX, searchY, MARGIN_ANGLE)) search_mode ++;
				break;
				   }
			case MOVE_TARGET:{
				if (mode_period >= 10.0f) search_mode ++;				// 10�b�Ԍo�߂��Ă������𖞂����Ȃ��ꍇ�́C���̃��[�h�Ɉڂ�D
																		// ��Q���Œ�~���Ă��܂����ƂɊւ���΍�
				if (!moveToPos(searchX, searchY, MARGIN_DIST_HUMAN)) search_mode ++;
				break;
				   }
			case INDICATE_FIND:{
				PlaySound("find.wav", NULL, SND_FILENAME | SND_ASYNC);
				search_mode ++;
				break;
				   }
			case INDICATE_FIND_WAIT:{
				stop();					// ��~
				if (mode_period >= 2.0f) search_mode ++;
				break;
				   }
			case TURN_RETURN:{
				if (!turnToPos(retX, retY, MARGIN_ANGLE)) search_mode ++;
				break;
				   }
			case MOVE_WAYPOINT:{
				if (!moveToPos(retX, retY, MARGIN_DIST_RETURN)) search_mode ++;
				break;
				   }
			case TURN_WAYPOINT:{
				if (!turnToPos(tarX, tarY, MARGIN_ANGLE)) search_mode ++;
				break;
				   }
			case FINISH:{
				data_no = 0;											// �f�[�^�̃N���A
				is_search_mode = 0;
				break;
				   }
		}
	}

	return res;
}

/*!
 * @brief ��~����i�T�����[�h�̂݁j
 *
 * @return 0
 */
int navi::stop()
{
	forwardSpeed = 0.0f;
	rotateSpeed  = 0.0f;

	return 0;
}

/*!
 * @brief �ڕW(x,y)�Ɍ�������
 *
 * @param[in] x �ڕW��x���W(m) �O���[�o�����W
 * @param[in] y �ڕW��y���W(m) �O���[�o�����W
 * @param[in] margin_angle �I������p�x�덷(rad)
 *
 * @return 1:�p�����C0:�I��
 */
int navi::turnToPos(float x, float y, float margin_angle)
{
	const float ROTATE_SPEED = 0.5f;
	
	int ret = 1;
	float ang = atan2(y - estY, x - estX);
	float ang_err = maxPI(ang - estThe);
	
	forwardSpeed = 0.0f;
	if (fabs(ang_err) > margin_angle){
		if (ang_err > 0.0f) rotateSpeed =  ROTATE_SPEED;
		else				rotateSpeed = -ROTATE_SPEED;
	} else {
		rotateSpeed = 0.0f;
		ret = 0;				// �^�X�N���I��
	}

	return ret;
}

/*!
 * @brief �ڕW(x,y)�ɋߊ��
 *
 * @param[in] x �ڕW��x���W(m) �O���[�o�����W
 * @param[in] y �ڕW��y���W(m) �O���[�o�����W
 * @param[in] margin_distance �I�����鋗��(m)
 *
 * @return 1:�p�����C0:�I��
 */
int navi::moveToPos(float x, float y, float margin_distance)
{
	const float FORWARD_SPEED = 0.15f;
	const float ROTATE_GAIN = 0.3f;
	
	int ret = 1;
	float dx = x - estX, dy = y - estY;
	float dist = sqrt(dx * dx + dy * dy);
	float ang = atan2(dy, dx);
	float ang_err = maxPI(ang - estThe);
	
	rotateSpeed = ang_err * ROTATE_GAIN;
	if (dist > margin_distance){
		forwardSpeed = FORWARD_SPEED;
	} else {
		forwardSpeed = 0.0f;
		rotateSpeed = 0.0f;
		ret = 0;
	}

	return ret;
}

/*!
 * @brief �T�����[�h���ǂ�����߂�(�j
 *
 * @return 0:�T�����[�h�łȂ��C1:�T�����[�h
 */
int navi::isSearchMode()
{
	return  is_search_mode;
}

/*!
 * @brief ���x�̎w�ߒl��߂�
 *
 * @param[out] front  �O��̑��x(m/s)
 * @param[out] radius ��]���a(m)
 *
 * @return 0
 */
int navi::getSpeed(float *forward, float *rotate)
{
	*forward = forwardSpeed;
	*rotate  = rotateSpeed;

	return 0;
}

/*!
 * @brief �O��̒T���Ώۂ���̋�����߂�
 *
 * @return �O��̒T���Ώۂ���̋���(m)
 */
float navi::distaceFromPreviousSearchPoint()
{
	float dx = estX - searchX;
	float dy = estY - searchY;

	return sqrt(dx * dx + dy * dy);
}

/*!
 * @brief �����[�g���[�h�ɂ���D
 *
 * @param[in] direction �����[�g�̕���
 *
 * @return 0
 */
int navi::setRerouteMode(int direction)
{
	is_reroute_mode = 1;
	reroute_direction = direction;
	reroute_mode = 0;

	return 0;
}

int navi::isRerouteMode()
{
	return is_reroute_mode;
}

/*!
 * @brief �T���̃v���Z�X
 *
 * @return 1:�p�����C0:�I��
 */
int navi::rerouteProcess()
{
	LOG("reroute_process, reroute_mode : %d\n", reroute_mode);


	const float MARGIN_ANGLE = 0.1f;		// ���̃��[�h�Ɉڂ�p�x�덷(rad)
	const float MARGIN_DIST_HUMAN = 1.0f;	// �l�ɋ߂Â�����(m)
	const float MARGIN_DIST_RETURN = 0.1f;	// �o�H�ɖ߂�Ƃ��Ɏ��̃��[�h�Ɉڂ�ʒu�덷(m)

	enum SEQUENCE {
		INIT = 0,
		TURN_SIDE,
		MOVE_SIDE,
		TURN_FORWARD,
		MOVE_FORWARD,
		TURN_RETURN,
		MOVE_RETURN,
		TURN_WAYPOINT,
		FINISH
	};
	
	int res = 1;

	static long time_mode0 = 0;				// �O��mode��؂�ւ������̎���(ms)
	static int reroute_mode0 = -1;			// �O���search_mode
	if (reroute_mode != reroute_mode0){
		reroute_mode0 = reroute_mode;
		time_mode0 = timeGetTime();
	}
	float mode_period = (timeGetTime() - time_mode0) / 1000.0f;

	LOG("reroute_mode:%d\n", reroute_mode);
	if (mode_period > 6.0f) reroute_mode ++;
	if (mode_period < 1.0f){
		stop();
		rerouteX0 = estX, rerouteY0 = estY, rerouteThe0 = estThe;
	} else {
		switch(reroute_mode){
			case INIT:{
				reroute_mode ++;
				break;
					}
			case TURN_SIDE:{
				if (!turn90deg(reroute_direction, rerouteThe0, MARGIN_ANGLE)) reroute_mode ++;
				break;
					}
			case MOVE_SIDE:{
				if (!moveForward(0.282, rerouteX0, rerouteY0)) reroute_mode ++;
				break;
					}
			case TURN_FORWARD:{
				if (!turn90deg(-reroute_direction, rerouteThe0, MARGIN_ANGLE)) reroute_mode ++;
				break;
					}
			case MOVE_FORWARD:{
				if (is_need_stop) reroute_mode = TURN_SIDE;
				if (!moveForward(0.5, rerouteX0, rerouteY0)) reroute_mode ++;
				break;
					}
			case TURN_RETURN:{
				if (!turn90deg(-reroute_direction, rerouteThe0, MARGIN_ANGLE)) reroute_mode ++;
				break;
					}
			case MOVE_RETURN:{
				if (!moveForward(0.282, rerouteX0, rerouteY0)) reroute_mode ++;
				break;
					}
			case TURN_WAYPOINT:{
				if (!turn90deg(reroute_direction, rerouteThe0, MARGIN_ANGLE)) reroute_mode ++;
				break;
					}
			case FINISH:{
				data_no = 0;
				is_reroute_mode = 0;
				res = 0;
				break;
					}
		}
	}

	return res;
}

/*!
 * @brief 90�x��]����D
 *
 * @return 1:�p�����C0:�I��
 */
int navi::turn90deg(int direction, float the0, float margin_angle)
{
	const float ROTATE_SPEED = 0.5f;
	
	int ret = 1;
	float ang;
	if (direction == RIGHT){
		ang = the0 - M_PI / 2.0f;
	} else {
		ang = the0 + M_PI / 2.0f;
	}
	float ang_err = maxPI(ang - estThe);
	
	forwardSpeed = 0.0f;
	if (fabs(ang_err) > margin_angle){
		if (ang_err > 0.0f) rotateSpeed =  ROTATE_SPEED;
		else				rotateSpeed = -ROTATE_SPEED;
	} else {
		rotateSpeed = 0.0f;
		ret = 0;				// �^�X�N���I��
	}

	return ret;
}

/*!
 * @brief ���鋗���������s
 *
 * @return 1:�p�����C0:�I��
 */
int navi::moveForward(float length, float x0, float y0)
{
	const float FORWARD_SPEED = 0.15f;
	
	int ret = 1;
	float dx = estX - x0, dy = estY - y0;
	float dist = sqrt(dx * dx + dy * dy);
	
	rotateSpeed = 0;
	if (dist < length){
		forwardSpeed = FORWARD_SPEED;
	} else {
		forwardSpeed = 0.0f;
		rotateSpeed = 0.0f;
		ret = 0;
	}

	return ret;
}

/*!
 * @brief �����̌W�����Z�b�g����D
 *
 * @return 0
 */
int navi::setNeedStop(int is_need_stop)
{
	this->is_need_stop = is_need_stop;

	return 0;
}

