#pragma once
#include "dataType.h"

class detectTarget
{
public:
	detectTarget(void);								// �R���X�g���N�^
	~detectTarget(void);							// �f�X�g���N�^

private:
	// ���ˋ��x�f�[�^
	static const int MAX_INTENSITY_DATA = 10000;	//! ���ˋ��x�̃f�[�^�̍ő��
	int intensity_data_no;							//! ���ˋ��x�̃f�[�^�̌�
	pos_inten intensity_data[MAX_INTENSITY_DATA];	//! ���ˋ��x�̃f�[�^

	// �T���Ώۂ̌��
	static const int MAX_SLATE_POINT = 100;			//! �T���Ώۂ̌��̍ő��
	int slate_point_no;								//! �T���Ώۂ̌��̌�
	pos_slate slate_point[MAX_SLATE_POINT];			//! �T���Ώۂ̌��

	// �T���Ώ�
	static const int MAX_SEARCH_POINT = 100;		//! �T���_�̍ő��
	int search_point_no;							//! �T���_�̌�
	pos_slate search_point[MAX_SEARCH_POINT];		//! �T���_�̃f�[�^

	/*!
	 * @struct pos_integrate_T
	 * @brief ���W�_�����p�\����
	 */
	struct pos_integrate_T{
		pos pos;									// �ʒu�f�[�^
		int count;									// ���������_�̌�
	};
	typedef struct pos_integrate_T pos_integrate;	// ���W�_�����p�̌^
	pos_integrate integrated_point[MAX_INTENSITY_DATA];	// ���W�_�����p�̃f�[�^
	int integrated_point_no;						// ���W�_�����p�̃f�[�^�̌�

	float distance_xy2(pos p, pos q);				// �_�̋��������߂�(m)
	static int comp_inten(const void *c1, const void *c2);	// pos_inten�^�̃\�[�g�̂��߂̔�r�֐�
	static int comp_slate(const void *c1, const void *c2);	// pos_slate�^�̃\�[�g�̂��߂̔�r�֐�
	int integratePoints(pos_inten *p, int num, pos_integrate *q, int *num_pos_integrate, float radius);
													// ���ˋ��x�̑傫���_�𓝍�����

	int terminate;									//! �X���b�h��j���i1:�j��, 0:�p���j
	static DWORD WINAPI ThreadFunc(LPVOID lpParameter);		// �X���b�h�̃G���g���[�|�C���g
	DWORD WINAPI ExecThread();						// �ʃX���b�h�œ��삷��֐�
	HANDLE mutex;									//! �r������̃n���h��

public:
	int Init();										// ������
	int Close();									// �I������
	int addIntensityData(pos_inten *p, int num);	// ���ˋ��x�f�[�^�̃Z�b�g
	int getTargetPos(pos_slate *p, int *num);		// �T���Ώۂ̈ʒu�Ɗm���Z�b�g
	int getSearchPos(pos *p, pos self_loc, float radius);			// �T���͈͓��̒T���Ώۂ̈ʒu�i0:�T���Ώۖ����C1:�L��j
	int calculateIntensity();						// ���ˋ��x�f�[�^�̃Z�b�g����T���Ώۂ̌���I��
	int calculateSearchPoint();						// �T���Ώۃ|�C���g���v�Z
	int update();									// �����I�ȏ����i1s���x�j
};
