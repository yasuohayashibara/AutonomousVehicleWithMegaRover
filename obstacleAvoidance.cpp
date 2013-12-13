/*!
 * @class obstacleAvoidance
 * @brief ��Q��������s���N���X
 * @date 2013.11.09
 * @author Y.Hayashibara
 *
 * �ڂ̑O�ɏ�Q���������slow_down_factor�̒l��������(1->0)���Ē�~����D
 */

#include "stdafx.h"
#include "obstacleAvoidance.h"
#include <mmsystem.h>
#include <time.h>

/*!
 * @brief �R���X�g���N�^
 */
obstacleAvoidance::obstacleAvoidance():
is_obstacle(0), slow_down_factor(1.0f),
obstacle_detect_time(0), obstacle_detect_period(0),
is_reroute(0), reroute_direction(0), is_need_stop(0)
{
}

/*!
 * @brief �f�X�g���N�^
 */
obstacleAvoidance::~obstacleAvoidance()
{
}

/*!
 * @brief ������
 *
 * @return 0
 */
int obstacleAvoidance::Init()
{
	return 0;
}

/*!
 * @brief �I������
 *
 * @return 0
 */
int obstacleAvoidance::Close()
{
	return 0;
}

/*!
 * @brief ��Q�������邩�ǂ�����߂�
 *
 * @return 0:��Q������,1:��Q���L��
 */
int obstacleAvoidance::isObstacle()
{
	return is_obstacle;
}

/*!
 * @brief �����̔䗦��߂�
 * �����̔䗦�iSTOP_LENGTH�ȉ���0, SLOW_DOWN_LENGTH��1�j
 * �����ڕW���x�ɂ����邱�ƂŁC���X�ɒ�~����
 *
 * @return �����̔䗦(0�`1)
 */
float obstacleAvoidance::getSlowDownFactor()
{
	return slow_down_factor;
}

/*!
 * @brief ��Q���̈ʒu�f�[�^��ݒ�
 *
 * @param[in] p   ��Q���̈ʒu�f�[�^
 * @param[in] num ��Q���̈ʒu�f�[�^�̐�
 *
 * @return 0
 */
int obstacleAvoidance::setData(pos *p, int num)
{
	obs_num = min(num, MAX_OBS_NUM);
	pos *q = obs;
	for(int i = 0; i < obs_num; i ++){
		*q ++ = *p ++;
	}
	Update();

	return 0;
}

/*!
 * @brief �����[�g���邩�ǂ�����߂��D
 *
 * @param[out] direction ����������
 *
 * @return 0:�����[�g����C1:�����[�g���Ȃ��D
 */
int obstacleAvoidance::isReroute()
{
	return is_reroute;
}

int obstacleAvoidance::getRerouteDirection()
{
	return reroute_direction;
}


/*!
 * @brief ��Q������̏������s���D(setData���ɌĂяo�����)
 *
 * @return 0
 */
int obstacleAvoidance::Update()
{
	static const int FLAG_CLEAR_PERIOD = 2000;
	static long time0 = 0;
	static int min_len0 = SLOW_DOWN_LENGTH;		//! ��Q���̍ŒZ������ۑ�����ϐ��DFLAG_CLEAR_PERIOD(2�b)������Q���𔭌����Ȃ��ƃN���A

	int min_len;
	is_obstacle = isDetectObstacle(CENTER, &min_len);	// �O�̏�Q���̌��o
	
	float factor = (float)(min_len - STOP_LENGTH)/(SLOW_DOWN_LENGTH - STOP_LENGTH);
	if (is_obstacle && (factor < 0.1f)){
		is_need_stop = 1;
	} else {
		is_need_stop = 0;
	}

	// ��Q���̌��o�C�X�L�������J��Ԃ����߁C��x���o�����t���O���b���ێ�
	if (is_obstacle){	// ������ׂ���Q��������ꍇ
		// slow_down_factor STOP_LENGTH�ȉ���0, SLOW_DOWN_LENGTH��1
		if (min_len < min_len0) min_len0 = min_len;
		slow_down_factor = (float)(min_len0 - STOP_LENGTH)/(SLOW_DOWN_LENGTH - STOP_LENGTH);
		slow_down_factor = min(max(slow_down_factor, 0),1);
		
		// ��Q�������m���Ă��鎞�Ԃ����߂�
		if (slow_down_factor < 0.1){
			if (obstacle_detect_time == 0){
				obstacle_detect_time = timeGetTime();
			} else {
				obstacle_detect_period = (float)(timeGetTime() - obstacle_detect_time)/1000;
			}
		} else {
			obstacle_detect_time = 0;
			obstacle_detect_period = 0;
		}

		// �����[�g
		if ((!is_reroute) && (obstacle_detect_period > REROUTE_PERIOD)){
			obstacle_detect_time = 0;
			is_reroute = 1;
			if (!isDetectObstacle(RIGHT, &min_len)){
				reroute_direction = RIGHT;
			} else if (!isDetectObstacle(LEFT , &min_len)){
				reroute_direction = LEFT;
			} else {
				reroute_direction = RIGHT;
			}
		}
		time0 = timeGetTime();
	} else {						// ��Q���������ꍇ
		long time = timeGetTime();							// 2�b�ȏ�̊Ԋu�������ăt���O�̃N���A
		if ((time - time0) > FLAG_CLEAR_PERIOD){
			is_obstacle = 0;
			slow_down_factor = 1;
			obstacle_detect_time = 0;
			obstacle_detect_period = 0;
			min_len0 = SLOW_DOWN_LENGTH;
		}
	}
	
	return 0;
}

/*!
 * @brief ����̈�ɓ����Q���̌��̌v�Z
 * (x_min,y_min)-(x_max,y_max)�Ɋ܂܂��
 *
 * @param[in] x_min     �T������x���W�̍ŏ��l(mm) - ��
 * @param[in] y_min     �T������y���W�̍ŏ��l(mm) - �E
 * @param[in] x_max     �T������x���W�̍ő�l(mm) - �O
 * @param[in] y_max     �T������y���W�̍ő�l(mm) - ��
 * @param[in] nearest_x �T������y���W�̍ő�l(mm) - �O
 *
 * @return ��Q���̌�
 */
int obstacleAvoidance::getDataNum(int x_min, int y_min, int x_max, int y_max, int *nearest_x)
{
	int point_num = 0, min_len = 10000;
	for(int i = 0; i < obs_num; i ++){
		if ((obs[i].x >= x_min)&&(obs[i].x < x_max)&&(obs[i].y >= y_min)&&(obs[i].y <= y_max)){
			point_num ++;
			if (obs[i].x < min_len) min_len = obs[i].x;
		}
	}
	*nearest_x = min_len;

	return point_num;
}

/*!
 * @brief �E�C�����C���ɏ�Q�������邩���v������
 *
 * @param[in] right_center_left �E�ƒ����ƍ��̎w��(RIGHT=-1,CENTER=0,LEFT=+1)
 *
 * @return ��Q���̌�
 */
int obstacleAvoidance::isDetectObstacle(int right_center_left, int *min_x)
{
	static const int MIN_POINT = 5;		// ��Q���Ƃ݂Ȃ��ŏ��̃f�[�^���i�P��̃X�L�����Łj
	int point_num = 0, min_len = 10000, res = 0;
	
	if (right_center_left == CENTER){
		point_num = getDataNum(100, -(TREAD/2+MARGIN)      , SLOW_DOWN_LENGTH, (TREAD/2+MARGIN)      , &min_len);	// �����łP�䕪�T�������Ƃ��ɏ�Q���𔭌����邩�D
	} else if (right_center_left == RIGHT){
		point_num = getDataNum(100, -(TREAD/2+MARGIN)-TREAD, SLOW_DOWN_LENGTH, (TREAD/2+MARGIN)-TREAD, &min_len);	// �E�ɂP�䕪�T�������Ƃ��ɏ�Q���𔭌����邩�D	
	} else if (right_center_left == LEFT){
		point_num = getDataNum(100, -(TREAD/2+MARGIN)+TREAD, SLOW_DOWN_LENGTH, (TREAD/2+MARGIN)+TREAD, &min_len);	// ���ɂP�䕪�T�������Ƃ��ɏ�Q���𔭌����邩�D
	}
	if (point_num > MIN_POINT){
		res = 1;
		*min_x = min_len;
	}

	return res;
}

/*!
 * @brief �����[�g���I������D
 *
 * @return 0
 */
int obstacleAvoidance::finishReroute()
{
	is_reroute = 0;

	return 0;
}

/*!
 * @brief ��~���K�v�ȏ󋵂��ǂ�����߂�
 *
 * @return 1:��~���K�v�C0:�K�v���Ȃ�
 */
int obstacleAvoidance::isNeedStop()
{
	return is_need_stop;
}
