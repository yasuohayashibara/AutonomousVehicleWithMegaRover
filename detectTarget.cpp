/*!
 * @file  detectTarget.cpp
 * @brief ���΃`�������W�p�T���ΏێҌ��o�v���O����
 * @date 2013.10.31
 * @author Y.Hayashibara
 */

#include "StdAfx.h"
#include "detectTarget.h"

/*!
 * @brief �R���X�g���N�^
 */
detectTarget::detectTarget(void):
	intensity_data_no(0), slate_point_no(0), search_point_no(0),
	integrated_point_no(0), terminate(0)
{
}

/*!
 * @brief �f�X�g���N�^
 */
detectTarget::~detectTarget(void)
{
}

/*!
 * @brief ������
 *
 * @return 0
 */
int detectTarget::Init()									// ������
{
	mutex = CreateMutex(NULL, FALSE, _T("DETECT_TARGET_RESULT"));

	// ���x����̃X���b�h���J�n
	DWORD threadId;	
	HANDLE hThread = CreateThread(NULL, 0, ThreadFunc, (LPVOID)this, 0, &threadId); 
	// �X���b�h�̗D�揇�ʂ��グ��
	SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);

	return 0;
}

/*!
 * @brief �I������
 *
 * @return 0
 */
int detectTarget::Close()									// �I������
{
	terminate = 1;											// �T���Ώی��o�X���b�h�̒�~
	CloseHandle(mutex);
	return 0;
}

/*!
 * @brief ���ˋ��x�f�[�^���Z�b�g
 * �v�Z������O�ɁC�K�����͂���D
 *
 * @param[in] p   ���ˋ��x�f�[�^(m)�i���݂̃��[���h���W�n�j
 * @param[in] num ���ˋ��x�f�[�^�̌�
 *
 * @return 0
 */
int detectTarget::addIntensityData(pos_inten *p, int num)
{
	for(int i = 0; i < num; i ++){
		intensity_data[intensity_data_no ++] = p[i];
		if (intensity_data_no >= MAX_INTENSITY_DATA) break;
	}

	return 0;
}

/*!
 * @brief �T���Ώۂ̈ʒu�Ɗm����߂�
 *
 * @param[out] p   �T���Ώۂ̈ʒu�Ɗm���i���݂̃��[���h���W�n�j
 * @param[out] num �T���Ώۂ̌�
 *
 * @return 0
 */
int detectTarget::getTargetPos(pos_slate *p, int *num)
{
	WaitForSingleObject(mutex, INFINITE);
	for(int i = 0; i < slate_point_no; i ++){
		p[i] = slate_point[i];
	}
	*num = slate_point_no;
	ReleaseMutex(mutex);
	return 0;
}

/*!
 * @brief �ŋߖ_�̒T���Ώۂ̈ʒu
 *
 * @param[out] p   �ŋߖT�̒T���Ώۂ̈ʒu�i���݂̃��[���h���W�n�j
 * @param[in] self_loc   ���{�b�g�̌��݈ʒu�i���݂̃��[���h���W�n�j
 * @param[in] radius   �@���{�b�g�̒T���͈�(m)
 *
 * @return 0:�T���Ώۖ����C1:�T���ΏۗL��
 */
int detectTarget::getSearchPos(pos *p, pos self_loc, float radius)
{
	const float MIN_PROBABILITY = 0.2f;		//! ToDo: �m���̂������l�i�����������邱�ƂŁC�m�����オ��D���l��傫������ƌ�F��������D�j

	int res = 0;

	for(int i = 0; i < search_point_no; i ++){
		if (distance_xy2(search_point[i].pos, self_loc) < radius){
			if (search_point[i].probability > MIN_PROBABILITY){
				*p = search_point[i].pos;
				res = 1;
				break;
			}
		}
	}

	return res;
}


/*!
 * @brief ���ˋ��x����T���Ώۂ̌����v�Z����D
 *
 * @return 0
 */
int detectTarget::calculateIntensity()
{
	const float INTEGRATE_RADIUS = 1.0;		// �������锼�a(m)
	const int DETECT_MIN_NUM = 5;
	const int DETECT_MAX_NUM = 15;
	
	int num = 0;

	integratePoints(intensity_data, intensity_data_no, integrated_point, &integrated_point_no, INTEGRATE_RADIUS);
	intensity_data_no = 0;

	WaitForSingleObject(mutex, INFINITE);
	for(int i = 0; i < integrated_point_no; i++){
		if (integrated_point[i].count > DETECT_MIN_NUM){
			slate_point[num].pos = integrated_point[i].pos;
			slate_point[num].probability = min(max((float)(integrated_point[i].count - DETECT_MIN_NUM) / 
				(DETECT_MAX_NUM - DETECT_MIN_NUM), 0.0f), 1.0f);
			num ++;
		}
	}
	slate_point_no = num;
	ReleaseMutex(mutex);
	
	return 0;
}

/*!
 * @brief �T���Ώۃ|�C���g���v�Z����D
 *
 * @return 0
 */
int detectTarget::calculateSearchPoint()
{
	static const float INTEGRATE_RADIUS = 1.0f;		// �������锼�a(m)
	static const float UP_PROBABILITY   = 0.1f;
	static const float DOWN_PROBABILITY = 0.1f;

	int i,j;

	for(i = 0; i < slate_point_no; i ++){
		for(j = 0; j < search_point_no; j ++){
			if (distance_xy2(slate_point[i].pos, search_point[j].pos) <  INTEGRATE_RADIUS){
				float prob_search = search_point[j].probability;
				float prob_slate  = slate_point[i].probability;
				search_point[j].pos.x = (int)((search_point[j].pos.x * prob_search + slate_point[i].pos.x * prob_slate) / (prob_search + prob_slate));
				search_point[j].pos.y = (int)((search_point[j].pos.y * prob_search + slate_point[i].pos.y * prob_slate) / (prob_search + prob_slate));
				search_point[j].pos.z = (int)((search_point[j].pos.z * prob_search + slate_point[i].pos.z * prob_slate) / (prob_search + prob_slate));
				search_point[j].probability += (DOWN_PROBABILITY + UP_PROBABILITY * slate_point[i].probability);
				break;
			}
		}
		if (j == search_point_no){					// �����ꏊ���Ȃ������ꍇ�C�T���|�C���g��ǉ�
			if (search_point_no < MAX_SEARCH_POINT){
				search_point[search_point_no] = slate_point[i];
				search_point[search_point_no].probability = (DOWN_PROBABILITY + UP_PROBABILITY * slate_point[i].probability);
				search_point_no ++;
			}
		}
	}
	qsort(search_point, search_point_no, sizeof(struct pos_slate_T), comp_slate);
	
	for(i = 0; i < search_point_no; i ++){		// �m���������āC0�ȉ��ɂȂ�����T���Ώۂ���O��
		search_point[i].probability -= DOWN_PROBABILITY;
		if (search_point[i].probability <= 0.0f) break;
	}
	search_point_no = i;

	return 0;
}


/*!
 * @brief �����I�ȏ����i1s���x�j
 *
 * @return 0
 */
int detectTarget::update()
{
	calculateIntensity();
	calculateSearchPoint();

	return 0;
}

/*!
 * @brief ���������߂�
 *
 * @param[in]  p   ���W�_�P
 * @param[in]  p   ���W�_�Q
 *
 * @return ����(m)
 */
float detectTarget::distance_xy2(pos p, pos q){
	float dx = (p.x - q.x) / 1000.0f;
	float dy = (p.y - q.y) / 1000.0f;

	return (dx * dx + dy * dy);
}

/*!
 * @brief pos_inten�^�̃\�[�g�̂��߂̔�r�֐�
 *
 * @param[in] c1 ���������ʒu�P�̃|�C���^
 * @param[in] c2 ���������ʒu�Q�̃|�C���^
 *
 * @return 1:c1�̌��������C0:���������C-1:c2�̌������Ȃ�
 */
int detectTarget::comp_inten(const void *c1, const void *c2)
{
	const struct pos_integrate_T *p1 = (struct pos_integrate_T *)c1;
	const struct pos_integrate_T *p2 = (struct pos_integrate_T *)c2;

	if      (p2->count > p1->count) return 1;
	else if (p2->count < p1->count) return -1;
	else return 0;
}


/*!
 * @brief pos_slate�^�̃\�[�g�̂��߂̔�r�֐�
 *
 * @param[in] c1 ���������ʒu�P�̃|�C���^
 * @param[in] c2 ���������ʒu�Q�̃|�C���^
 *
 * @return 1:c1�̌��������C0:���������C-1:c2�̌������Ȃ�
 */
int detectTarget::comp_slate(const void *c1, const void *c2)
{
	const struct pos_slate_T *p1 = (struct pos_slate_T *)c1;
	const struct pos_slate_T *p2 = (struct pos_slate_T *)c2;

	if      (p2->probability > p1->probability) return 1;
	else if (p2->probability < p1->probability) return -1;
	else return 0;
}


/*!
 * @brief ���ˋ��x�̑傫���_�𓝍�����D
 *
 * @param[in]  p   �������l�ȏ�̔��ˋ��x�f�[�^(m)�i���݂̃��[���h���W�n�j
 * @param[in]  num �������l�ȏ�̔��ˋ��x�f�[�^�̌�
 * @param[out] q   �����������ˋ��x�̑傫�Ȉʒu(m)�i���݂̃��[���h���W�n�j
 * @param[out] num_pos_integrate �����������ˋ��x�̑傫�Ȉʒu�̌�
 * @param[in]  radius �������锼�a
 *
 * @return 0
 */
int detectTarget::integratePoints(pos_inten *p, int num, pos_integrate *q, int *num_pos_integrate, float radius)
{
	int i, j, n, k = 0;
	float radius2 = radius * radius;
	
	for(i = 0; i < num; i ++){
		q[i].pos = p[i].pos;
		q[i].count = 1;
	}
	n = num;
	for(i = 0; i < n; i ++){
		if (!q[i].count) continue;
		for(j = i + 1; j < n; j ++){
			if (!q[j].count) continue;
			if (distance_xy2(q[i].pos, q[j].pos) <= radius2){
				int c = q[i].count;
				q[i].pos.x = (q[i].pos.x * c + q[j].pos.x) / (c + 1);
				q[i].pos.y = (q[i].pos.y * c + q[j].pos.y) / (c + 1);
				q[i].pos.z = (q[i].pos.z * c + q[j].pos.z) / (c + 1);
				q[i].count ++;
				q[j].count = 0;
			}
		}
	}

	// ���ɂ��\�[�e�B���O
	qsort(q, n, sizeof(struct pos_integrate_T), comp_inten);
	for(i = 0; i < n; i ++){				// �f�[�^��O�ɋl�߂�
		if (!q[i].count) break;
	}
	*num_pos_integrate = i;

	return 0;
}

/*!
 * @brief �X���b�h�̃G���g���[�|�C���g
 *
 * @param[in] lpParameter �C���X�^���X�̃|�C���^
 * 
 * @return S_OK
 */
DWORD WINAPI detectTarget::ThreadFunc(LPVOID lpParameter) 
{
	return ((detectTarget*)lpParameter)->ExecThread();
}

/*!
 * @brief �ʃX���b�h�œ��삷��֐�
 * ���{�b�g�̐����ʃX���b�h�ōs���D
 *
 * @return S_OK
 */
DWORD WINAPI detectTarget::ExecThread()
{
	while(!terminate){
		update();
		Sleep(1000);
	}
	return S_OK; 
}
