// URG.cpp : �����t�@�C��

#include "stdafx.h"
#include "urg3D.h"
#include "logger.h"

#define URG_PORT 2

/*!
 * @class urg3D
 * @brief URG���g�p����3D��Q���f�[�^�����o����N���X
 * @author Y.Hayashibara
 */

/*!
 * @brief �R���X�g���N�^
 */
urg3D::urg3D():
tilt_low(0), tilt_high(0), tilt_period(1.0), num(0),terminate(0)
{
}

/*!
 * @brief �f�X�g���N�^
 */
urg3D::~urg3D()
{
}

/*!
 * @brief �������i�ŏ��ɂP��Ăяo���j
 *
 * @return 0
 */
int urg3D::Init()
{
	// �r������
	mutex = CreateMutex(NULL, FALSE, NULL);

	// URG�̏����ݒ�
	if (!urg.Init(URG_PORT)) AfxMessageBox("Cannot communicate URG0");// UHG�̏����ݒ�

	// RS405CB�̏����ݒ�
	hComm = CommOpen( COM_PORT );				// �ʐM�|�[�g���J��
	RSTorqueOnOff( hComm, 1 );					// �g���N��ON����
	Sleep(100);									// �����҂�
	RSMove( hComm, SERVO_OFFSET * 10, 200 );	// 2�b�����ď����p���Ɉړ�
	Sleep(3000);								// 3�b�҂�

	// ���x����̃X���b�h���J�n
	DWORD threadId;								// �X���b�h ID	
	HANDLE hThread = CreateThread(NULL, 0, ThreadFunc, (LPVOID)this, 0, &threadId); 

	return 0;
}

/*!
 * @brief �I�������i�Ō�ɂP��Ăяo���j
 *
 * @return 0
 */
int urg3D::Close()
{
	terminate = 1;
	CloseHandle(mutex);

	// RS405CB�̏I������
	RSMove( hComm, SERVO_OFFSET * 10, 200 );	// 2�b�����ď����p���Ɉړ�
	Sleep(3000);								// 3�b�҂�
	RSTorqueOnOff( hComm, 0 );					// �g���N��OFF����
	CommClose( hComm );							// �ʐM�|�[�g�����

	// URG�̏I������
	urg.Close();

	return 0;
}

/*!
 * @brief URG�̃f�J���g���W�n�ł̏�Q���f�[�^���擾
 *
 * @param[in] p �ʒu�f�[�^��ۑ�����z��
 * @param[in] max_no �f�[�^�̍ő���i�ʒu�f�[�^�̔z��̍ő�l�j
 * 
 * @return �f�[�^�̌�
 */
int urg3D::GetAllData(pos_inten *p, int max_no)
{
	WaitForSingleObject(mutex, INFINITE); 
	int no = min(num, max_no);
	for(int i = 0; i < no; i++){
		p[i].pos.x     = upos_inten[i].pos.x;
		p[i].pos.y     = upos_inten[i].pos.y;
		p[i].pos.z     = upos_inten[i].pos.z;
		p[i].intensity = upos_inten[i].intensity;
	}
	num -= no;
	ReleaseMutex(mutex);

	return no;
}

/*!
 * @brief �������w�肵��urg�̃f�J���g���W�n�ł̏�Q���f�[�^���擾
 *
 * @param[in] low �擾����f�[�^�̍ŏ�����
 * @param[in] high �擾����f�[�^�̍ő卂��
 * @param[in] p �ʒu�f�[�^��ۑ�����z��
 * @param[in] max_no �f�[�^�̍ő���i�ʒu�f�[�^�̔z��̍ő�l�j
 * 
 * @return �f�[�^�̌�
 */
int urg3D::GetSelectedData(int low, int high, pos *p, int max_no)
{
	int ret = 0, i;

	WaitForSingleObject(mutex, INFINITE); 
	for(i = 0; ((i < num)&&(ret < max_no)); i++){
		if ((upos_inten[i].pos.z >= low)&&(upos_inten[i].pos.z <= high)){
			p[ret ++] = upos_inten[i].pos;
		}
	}
	num -= i;
	ReleaseMutex(mutex);

	return ret;
}

/*!
 * @brief �Q�̍������w�肵��urg�̃f�J���g���W�n�ł̏�Q���f�[�^���擾
 *
 * @param[in] low1 �擾����f�[�^�P�̍ŏ�����
 * @param[in] high1 �擾����f�[�^�P�̍ő卂��
 * @param[in] low2 �擾����f�[�^�Q�̍ŏ�����
 * @param[in] high2 �擾����f�[�^�Q�̍ő卂��
 * @param[in] p1 �ʒu�f�[�^�P��ۑ�����z��
 * @param[in] p2 �ʒu�f�[�^�Q��ۑ�����z��
 * @param[out] no1 �f�[�^�P�̌�
 * @param[out] no2 �f�[�^�Q�̌�
 * @param[in] max_no1 �f�[�^�P�̍ő���i�ʒu�f�[�^�̔z��̍ő�l�j
 * @param[in] max_no2 �f�[�^�Q�̍ő���i�ʒu�f�[�^�̔z��̍ő�l�j
 * 
 * @return �f�[�^�̌�
 */
int urg3D::Get2SelectedData(int low1, int high1, pos *p1, int *no1, int max_no1,
							int low2, int high2, pos *p2, int *no2, int max_no2)
{
	int n1 = 0, n2 = 0, i;

	WaitForSingleObject(mutex, INFINITE); 
	for(i = 0; ((i < num)&&(n1 < max_no1)&&(n2 < max_no2)); i++){
		if ((upos_inten[i].pos.z >= low1)&&(upos_inten[i].pos.z <= high1)){
			p1[n1 ++] = upos_inten[i].pos;
		}
		if ((upos_inten[i].pos.z >= low2)&&(upos_inten[i].pos.z <= high2)){
			p2[n2 ++] = upos_inten[i].pos;
		}
	}
	*no1 = n1, *no2 = n2;
	num -= i;
	ReleaseMutex(mutex);

	return 0;
}

/*!
 * @brief �R�̍������w�肵��urg�̃f�J���g���W�n�ł̏�Q���f�[�^���擾
 * �P�Ɋւ��ẮC���ˋ��x���擾����D
 *
 * @param[in] low1 �擾����f�[�^�P�̍ŏ�����
 * @param[in] high1 �擾����f�[�^�P�̍ő卂��
 * @param[in] p1 �ʒu�f�[�^�P��ۑ�����z��
 * @param[out] no1 �f�[�^�P�̌�
 * @param[in] max_no1 �f�[�^�P�̍ő���i�ʒu�f�[�^�̔z��̍ő�l�j
 * @param[in] low2 �擾����f�[�^�Q�̍ŏ�����
 * @param[in] high2 �擾����f�[�^�Q�̍ő卂��
 * @param[in] p2 �ʒu�f�[�^�Q��ۑ�����z��
 * @param[out] no2 �f�[�^�Q�̌�
 * @param[in] max_no2 �f�[�^�Q�̍ő���i�ʒu�f�[�^�̔z��̍ő�l�j
 * @param[in] low3 �擾����f�[�^�R�̍ŏ�����
 * @param[in] high3 �擾����f�[�^�S�̍ő卂��
 * @param[in] p3 �ʒu�f�[�^�R��ۑ�����z��i���ˋ��x���܂ށj
 * @param[out] no3 �f�[�^�R�̌�
 * @param[in] max_no3 �f�[�^�R�̍ő���i�ʒu�f�[�^�̔z��̍ő�l�j
 * @param[in] min_intensity ���ˋ��x�̍ŏ��l
 * 
 * @return �f�[�^�̌�
 */
int urg3D::Get3SelectedData(int low1, int high1, pos *p1, int *no1, int max_no1,
							int low2, int high2, pos *p2, int *no2, int max_no2,
							int low3, int high3, pos_inten *p3, int *no3, int max_no3, int min_intensity)
{
	int n1 = 0, n2 = 0, n3 = 0, i;

	WaitForSingleObject(mutex, INFINITE); 
	for(i = 0; ((i < num)&&(n1 < max_no1)&&(n2 < max_no2)); i++){
		if ((upos_inten[i].pos.z >= low1)&&(upos_inten[i].pos.z <= high1)){
			p1[n1 ++] = upos_inten[i].pos;
		}
		if ((upos_inten[i].pos.z >= low2)&&(upos_inten[i].pos.z <= high2)){
			p2[n2 ++] = upos_inten[i].pos;
		}
		if ((upos_inten[i].pos.z >= low3)&&(upos_inten[i].pos.z <= high3)&&
			(upos_inten[i].intensity > min_intensity)){
			p3[n3 ++] = upos_inten[i];
		}
	}
	*no1 = n1, *no2 = n2, *no3 = n3;
	num -= i;
	ReleaseMutex(mutex);

	return 0;
}

/*!
 * @brief �`���g�A���O���̓����̐ݒ�
 *
 * @param[in] low �ŏ��p�x(deg) -90�`90
 * @param[in] high �ő�p�x(deg) -90�`90
 * @param[in] period ����(sec) low->high, high->low�ɗv���鎞��
 * 
 * @return 0
 */
int urg3D::SetTiltAngle(int low, int high, float period)
{
	if ((low > high)||(period < 0.0f)||(low < -90)||(high > 90)) return -1;
	tilt_low    = low;		// deg
	tilt_high   = high;		// deg
	tilt_period = period;	// sec

	return 0;
}

/*!
 * @brief �X���b�h�̃G���g���[�|�C���g
 *
 * @param[in] lpParameter �C���X�^���X�̃|�C���^
 * 
 * @return S_OK
 */
DWORD WINAPI urg3D::ThreadFunc(LPVOID lpParameter) 
{
	return ((urg3D*)lpParameter)->ExecThread();
}

/*!
 * @brief �ʃX���b�h�œ��삷��֐�
 * Update()��50ms���ɌĂяo���Ă���D
 *
 * @return S_OK
 */
DWORD WINAPI urg3D::ExecThread()
{
	while(!terminate){
		Update();
		Sleep(50);
	}
	return S_OK; 
}


/*!
 * @brief ����I(50ms)�ɌĂяo�����֐�
 * 1)�T�[�{���[�^�̐���
 * 2)�T�[�{���[�^�̊p�x�̎擾
 * 3)URG�̃f�[�^���擾
 * 4)�f�J���g���W�n�̐�L�f�[�^�ɕϊ����Ĕz��ɕۑ�
 *
 * @return 0
 */
int urg3D::Update()
{
	static int is_first = 1;
	static int length[urg.n_data], intensity[urg.n_data];
	static pos p[urg.n_data];
	static int is_up = 1, count = 1000;

	count ++;
	if (count >= (tilt_period * 1000 / 50)){
		if (is_up){
			RSMove(hComm, (tilt_high + SERVO_OFFSET) * 10, (int)(tilt_period * 100));
		} else {
			RSMove(hComm, (tilt_low  + SERVO_OFFSET) * 10, (int)(tilt_period * 100));
		}
		RSStartGetAngle(hComm);				// tilt�p�x�̎擾���n�߂�D
		is_up ^= 1;
		count = 0;
		return 0;
	}

	if (is_first){
		RSStartGetAngle(hComm);				// tilt�p�x�̎擾���n�߂�D
		urg.StartMeasure();
		is_first = 0;
		return 0;
	}
	
	int n = 0;
	float tilt_angle = RSGetAngle(hComm)/10.0f - SERVO_OFFSET;	// deg
	if (urg.n_data == urg.GetData(length, intensity)){
		n = urg.TranslateCartesian(tilt_angle, length, p);
	}
	WaitForSingleObject(mutex, INFINITE);	// mutex�̊J�n
	pos *q = p;
	int *r = intensity;
	for(int i = 0; (i < n)&&(num < MAX_NUM);i ++){
		if ((q->x != 0)||(q->y != 0)||(q->z != 0)){
			upos_inten[num   ].pos       = *q ++;
			upos_inten[num ++].intensity = *r ++;
		} else {
			q ++;
		}
	}
	ReleaseMutex(mutex);					// mutex�̏I��

	// Log�ɏ����o��
	LOG("tilt_angle:%f\n", tilt_angle);
	LOG("\n");								// ���Ԃ�\�����邽�� 
	for(int i = 0; i < num ;i ++){
		LOG_WITHOUT_TIME("urg:(%d,%d,%d),intensity:%d\n",
			upos_inten[i].pos.x, upos_inten[i].pos.y, upos_inten[i].pos.z, upos_inten[i].intensity); 
	}

	RSStartGetAngle(hComm);					// tilt�p�x�̎擾���J�n
	
	return 0;
}

/*!
 * @brief URG3D�̃f�[�^���N���A����D
 * 
 * @return 0
 */
int urg3D::ClearData()
{
	WaitForSingleObject(mutex, INFINITE); 
	num = 0;
	ReleaseMutex(mutex);
	
	return 0;
}