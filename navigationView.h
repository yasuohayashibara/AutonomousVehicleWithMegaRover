#pragma once

// CnavigationView

#include "URG.h"
#include "dataType.h"

class CnavigationView : public CStatic
{
	DECLARE_DYNAMIC(CnavigationView)

public:
	CnavigationView();
	virtual ~CnavigationView();

protected:
	DECLARE_MESSAGE_MAP()


private:
	// �v���_�̕\��
	static const int MAX_DATA_NUM = 10000;
	int data_num;
	pos data_pos[MAX_DATA_NUM];

	// �Q�ƃf�[�^�̕\���@(play���[�h�Ŏg�p)
	static const int MAX_REF_DATA_NUM = 10000;
	int ref_data_num;
	pos ref_pos[MAX_REF_DATA_NUM];

	// ���ˋ��x�f�[�^�̕\��
	static const int MAX_INTEN_DATA_NUM = 10000;
	int inten_data_num;
	pos_inten inten_pos[MAX_INTEN_DATA_NUM];

	// �T���Ώۂ̌��f�[�^�̕\��
	static const int MAX_SLATE_DATA_NUM = 100;
	int slate_data_num;
	pos_slate slate_pos[MAX_SLATE_DATA_NUM];

	// RGB�\���p�\����
	struct bitmap_rgb_T{
		unsigned char blue;
		unsigned char green;
		unsigned char red;
		unsigned char alpha;
	} rgb_thre[640*480];
	
	// �I�h���g���̕\��
	static const int MAX_ODO_NUM = 100;
	struct odometory odo[MAX_ODO_NUM];
	int odo_num;
	long time0;

	// �S�[���̕\���@(play���[�h�Ŏg�p)
	int is_target_view;
	float tarX;				// m
	float tarY;				// m
	float tarThe;			// rad

	// �p�[�e�B�N���̕\���@(play���[�h�Ŏg�p)
	static const int MAX_PARTICLE_NUM = 1000;	// �ő�1000�\��
	int particle_num;
	struct particle_T particle[MAX_PARTICLE_NUM];

	// ���[���h���W����f�B�Y�v���C�̍��W�ɕϊ� (�P�ʂ�mm)
	int translatePos(int worldX, int worldY, int worldX0, int worldY0, int *dispX, int *dispY);
	int center_x, center_y;								// �r���[�̂̒��S(dot)
	int disp_x, disp_y;									// �r���[�̃T�C�Y(dot)
	int step;											// waypoint�̔ԍ�
	float coincidence;									// ��v�x(0-1)
	int is_record, is_play;								// �ۑ��C�Đ�
	float ratio;										// ��ʂ̃T�C�Y�̔䗦(m->dot)

public:
	afx_msg void OnPaint();								// �ĕ`��
	int setData(pos *p, int num);						// ��Q���̈ʒu�f�[�^��ݒ�
	int setRefData(pos *p, int num);					// �Q�Ƃ����Q���̈ʒu�f�[�^�̐ݒ�
	int setIntensityData(pos_inten *p, int num);		// ���ˋ��x�̃f�[�^�̐ݒ�
	int setSlatePoint(pos_slate *p, int num);			// �T���Ώۂ̌��̐ݒ�
	int setOdometory(float x, float y, float the);		// �I�h���g���̐ݒ�
	int setTargetPos(float x, float y, float the);		// waypoint�̐ݒ�
	int setParticle(struct particle_T *p, int num);		// �p�[�e�B�N���̐ݒ�
	int setStep(int step);								// waypoint�̐��̐ݒ�
	int setCoincidence(float coincidence);				// ��v�x�̐ݒ�
	int setStatus(int is_record, int is_play);			// ���(�ۑ����[�h�C�Đ����[�h)�̐ݒ�
};
