#pragma once
#include "comm.h"
#include "dataType.h"

// CURG
class CURG
{

public:
	CURG();								// �R���X�g���N�^
	virtual ~CURG();					// �f�X�g���N�^

public:
	static const int n_data = 641;		// URG�Ŏ擾����f�[�^�̌� (10�`170deg)
	CComm comm;							// �ʐM�̃N���X

	int Init(int com_port);				// �����ݒ�
	int Close();						// �I������
	int StartMeasure();					// URG�̌v���J�n
	int GetData(int length[n_data], int intensity[n_data]);
										// ��M�o�b�t�@�ɂ��܂��������f�[�^���擾����
	int TranslateCartesian(float tilt, int data[n_data], pos p[n_data]);
										// �f�J���g���W�n�ւ̕ϊ�
};
