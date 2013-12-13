#include "stdafx.h"
#include <math.h>
#include <mmsystem.h>
#include "memoryMap.h"
#include "megaRover.h"
#include "Library/WRC003LVHID.h"

#define	M_PI	3.14159f

#pragma	comment(lib,"Library/CWRC003LVHID.lib")

/*!
 * @class megaRover
 * @brief Mega Rover�𐧌䂷�邽�߂̃N���X
 * @author Y.Hayashibara
 */

/*!
 * @brief �R���X�g���N�^
 */
megaRover::megaRover():
is_speed_control_mode(0), refSpeedRight(0), refSpeedLeft(0),terminate(0),
odoX(0), odoY(0), odoThe(0),deltaL(0),deltaR(0), mutex(NULL), comMutex(NULL),
#ifdef MEGA_ROVER_1_1
	MAX_SPEED(0.625f)
#else
	MAX_SPEED(0.3)
#endif
{
}

/*!
 * @brief �f�X�g���N�^
 */
megaRover::~megaRover()
{
}

/*!
 * @brief ������
 *
 * @return 0
 */
int megaRover::init()
{
	if(!CWRC_Connect()) return -1;

	// �r������
	mutex = CreateMutex(NULL, FALSE, _T("MEGA_ROVER_ODOMETORY"));
	comMutex = CreateMutex(NULL, FALSE, _T("MEGA_ROVER_COM_MUTEX"));

	// �G���R�[�_�̏�����
	SetMem_UByte(Flag, 0x02);	// �G���R�[�_��ON�ɂ���
	CWRC_WriteExecute(FALSE);

	SetMem_SWord(EncoderA, 0);	// ���݂̃G���R�[�_�l��0�ɖ߂�
	SetMem_SWord(EncoderB, 0);
	CWRC_WriteExecute(FALSE);

	// ���x����̃X���b�h���J�n
	DWORD threadId;	
	HANDLE hThread = CreateThread(NULL, 0, ThreadFunc, (LPVOID)this, 0, &threadId); 
	// �X���b�h�̗D�揇�ʂ��グ��
	SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
	
	return 0;
}

/*!
 * @brief �I������
 *
 * @return 0
 */
int megaRover::close()
{
	terminate = 1;				// ���x����X���b�h�̒�~
	CloseHandle(mutex);
	CloseHandle(comMutex);
	setMotor(0, 0);				// ���{�b�g���~�߂�
	servoOn(0);					// ���[�^��OFF�ɂ���
	CWRC_Disconnect();			// ���{�b�g�Ƃ̒ʐM��ؒf����

	return 0;
}

/*!
 * @brief �T�[�{ON/OFF
 *
 * @param[in] gain 0:OFF, 1-255:on (servo gain)
 *
 * @return 0
 */
int megaRover::servoOn(int gain)
{
	// ���[�^��ON�ɂ��邽�߂ɂ́A�������}�b�v��0x04�iMode�j��1�ɁA0x08,0x09�i���E�ԗւ̃Q�C���j�����ȏ�̒l�ɏ��������܂�
	SetMem_UByte(CpuMode     , gain > 0);
	SetMem_UByte(MotorGainCh1, gain);
	SetMem_UByte(MotorGainCh2, gain);
	if (comMutex == NULL) return -1;
	WaitForSingleObject(comMutex, INFINITE); 
	CWRC_WriteExecute(FALSE);
	ReleaseMutex(comMutex);

	return 0;
}

/*!
 * @brief �����x�̐ݒ�
 * �}�ȉ�������h�~���邽��
 *
 * @param[in] right �E�z�C�[���̉����x(m/s^2)
 * @param[in] left  ���z�C�[���̉����x(m/s^2)
 *
 * @return 0
 */
int megaRover::setDelta(int right, int left)
{
	deltaR = right;
	deltaL = left ;

	return 0;
}

/*!
 * @brief ���[�^���x�̐ݒ�
 *
 * @param[in] right �E�z�C�[���̃g���N�@(-1000~1000)
 * @param[in] left  ���z�C�[���̃g���N�@(-1000~1000)
 *
 * @return 0
 */
int megaRover::setMotor(int right, int left)
{
	static const int limit = 1000;
	static int right0, left0;
	if (deltaR != 0){
		if ((right - right0) > 0){
			right = min(right0 + deltaR, right);
		} else if ((right - right0) < 0){
			right = max(right0 - deltaR, right);
		}
	}
	if (deltaL != 0){
		if ((left - left0) > 0){
			left = min(left0 + deltaL, left);
		} else if ((left - left0) < 0){
			left = max(left0 - deltaL, left);
		}
	}
	
	right = min(max(right, -limit), limit);
	left  = min(max(left,  -limit), limit);

	if (comMutex == NULL) return -1;
	WaitForSingleObject(comMutex, INFINITE); 
//	SetMem_SWord(MotorSpeedCh1, (int)(right * 0.93f));	// ToDo �o�����X�����
	SetMem_SWord(MotorSpeedCh1, (int)(right * 1.00f));	// ToDo �o�����X�����
	SetMem_SWord(MotorSpeedCh2, (int)(left  * 1.00f));
	CWRC_WriteExecute(FALSE);
	ReleaseMutex(comMutex);

	right0 = right;
	left0  = left;

	return 0;
}

/*!
 * @brief �G���R�[�_�̒l�̎擾
 *
 * @param[out] right �E�z�C�[���̃G���R�[�_�̒l
 * @param[out] left  ���z�C�[���̃G���R�[�_�̒l
 *
 * @return 0
 */
int megaRover::getEncoder(unsigned int *right, unsigned int *left)
{
	// �G���R�[�_�̌��ݒl���擾
	if (comMutex == NULL) return -1;
	WaitForSingleObject(comMutex, INFINITE); 
	CWRC_ReadMemMap(EncoderA, 4);
	CWRC_ReadExecute();

	*right = (unsigned short)GetMem_SWord(EncoderA);
	*left  = (unsigned short)GetMem_SWord(EncoderB);
	ReleaseMutex(comMutex);

	return 0;
}

/*!
 * @brief ���x���䃂�[�h�̐ݒ�
 *
 * @param[in] is_on 1:���x���䃂�[�h�C0:����
 *
 * @return 0
 */
int megaRover::setSpeedControlMode(int is_on)
{
	is_speed_control_mode = is_on;

	return 0;
}

/*!
 * @brief ���x�̐���
 *
 * @param[in] front �O������̑��x(m/s)
 * @param[in] rotate �p���x(rad/s)
 *
 * @return 0
 */
int megaRover::setSpeed(float front, float rotate)
{
	refSpeedRight = front + rotate * (float)TREAD / 1000.0f / 2.0f;
	refSpeedLeft  = front - rotate * (float)TREAD / 1000.0f / 2.0f;

	float maxSpeed = max(fabs(refSpeedRight), fabs(refSpeedLeft));
	if (maxSpeed > MAX_SPEED){
		refSpeedRight *= (MAX_SPEED / maxSpeed);						// ������ꍇ�́C��]���a��ς��Ȃ��悤�ɗ��������䗦�ő��x��������D
		refSpeedLeft  *= (MAX_SPEED / maxSpeed);
	}

	return 0;
}

/*!
 * @brief �O��̑��x�Ɖ�]���a
 *
 * @param[in] front  �O��̑��x(m/s)
 * @param[in] radius ��]���a(m)
 *
 * @return 0
 */
int megaRover::setArcSpeed(float front, float radius)
{
	if (radius != 0.0f){
		float w = front / radius;
		refSpeedRight = (radius + (float)TREAD / 1000.0f / 2.0f) * w;
		refSpeedLeft  = (radius - (float)TREAD / 1000.0f / 2.0f) * w;
	}

	float maxSpeed = max(fabs(refSpeedRight), fabs(refSpeedLeft));
	if (maxSpeed > MAX_SPEED){
		refSpeedRight *= (MAX_SPEED / maxSpeed);					// ������ꍇ�́C��]���a��ς��Ȃ��悤�ɗ��������䗦�ő��x��������D
		refSpeedLeft  *= (MAX_SPEED / maxSpeed);
	}

	return 0;
}

/*!
 * @brief �I�h���g���̎擾
 *
 * @param[out] x   �I�h���g����x���W(m)
 * @param[out] y   �I�h���g����y���W(m)
 * @param[out] the �I�h���g���̊p�x(rad)
 * @param[in]  is_clear �I�h���g���̒l���N���A���邩�̃t���O(1:�N���A,0:�N���A���Ȃ�)
 *
 * @return 0
 */
int megaRover::getOdometory(float *x, float *y, float *the, int is_clear)
{
	if (mutex == NULL) return -1;
	WaitForSingleObject(mutex, INFINITE);
	*x   = odoX;
	*y   = odoY;
	*the = odoThe;
	if (is_clear){
		odoX = odoY = odoThe = 0;
	}
	ReleaseMutex(mutex);

	return 0;
}

/*!
 * @brief ���E�̃z�C�[���̑��x�̎擾
 *
 * @param[out] rightSpeed	�E�̃z�C�[���̑��x(m)
 * @param[out] leftSpeed	���̃I�C�[���̑��x(m)
 *
 * @return 0
 */
int megaRover::getSpeed(float *rightSpeed, float *leftSpeed)
{
	*rightSpeed = speedRight;
	*leftSpeed  = speedLeft ;
	return 0;
}

/*!
 * @brief �X���b�h�̃G���g���[�|�C���g
 *
 * @param[in] lpParameter �C���X�^���X�̃|�C���^
 * 
 * @return S_OK
 */
DWORD WINAPI megaRover::ThreadFunc(LPVOID lpParameter) 
{
	return ((megaRover*)lpParameter)->ExecThread();
}

/*!
 * @brief �ʃX���b�h�œ��삷��֐�
 * ���{�b�g�̐����ʃX���b�h�ōs���D
 *
 * @return S_OK
 */
DWORD WINAPI megaRover::ExecThread()
{
	while(!terminate){
		Update();
		Sleep(20);
	}
	return S_OK; 
}

/*!
 * @brief ����I(20ms)�ɌĂяo���֐�
 *
 * @return 0
 */
int megaRover::Update()
{
	const float coef = 1.0f;					// �v�����̍�����

	const int MAX_SHORT = 256 * 256;
	static unsigned int r0 = 0, l0 = 0;
	static unsigned long time0 = 0;
	static const float kp = 50000.0f, ki = 1000.0f;
	unsigned int r = 0, l = 0;
	int rd, ld;
	float right, left;
	float v, w;

	getEncoder(&r, &l);
	rd =    r - r0;
	ld  = -(int)(l - l0);	// ���̓}�C�i�X���O�i
	r0 = r, l0 = l;
	if (rd >= ( MAX_SHORT / 2)) rd -= MAX_SHORT;
	if (rd <= (-MAX_SHORT / 2)) rd += MAX_SHORT;
	if (ld >= ( MAX_SHORT / 2)) ld -= MAX_SHORT;
	if (ld <= (-MAX_SHORT / 2)) ld += MAX_SHORT;
#ifdef MEGA_ROVER_1_1
	right = (float)rd / (13 * 71.2f * 4) * 150 * M_PI / 1000.0f;		// �E�ԗւ̈ړ���(m)
	left  = (float)ld / (13 * 71.2f * 4) * 150 * M_PI / 1000.0f * coef;	// ���ԗւ̈ړ���(m)
#else
	right = (float)rd / (48 * 104 * 4) * 150 * M_PI / 1000.0f;			// �E�ԗւ̈ړ���(m)
	left  = (float)ld / (48 * 104 * 4) * 150 * M_PI / 1000.0f * coef;	// ���ԗւ̈ړ���(m)
#endif
	v = (right + left) / 2.0f;						// �O��̑��x
	w = (right - left) / (TREAD / 1000.0f);			// ��]���x

	if (mutex == NULL) return -1;
	WaitForSingleObject(mutex, INFINITE);
	odoX += v * cos(odoThe);
	odoY += v * sin(odoThe);
	odoThe += w;

	if (odoThe >  M_PI) odoThe -= 2.0f*M_PI;
	if (odoThe < -M_PI) odoThe += 2.0f*M_PI;
	ReleaseMutex(mutex);

	
	if (is_speed_control_mode){
		static int is_first = 1;
		if (is_first){
			time0 = timeGetTime();
			is_first = 0;
		} else {
			unsigned long time = timeGetTime();
			static float errIntRight = 0, errIntLeft = 0; 
			float period = (time - time0) / 1000.0f;
			time0 = time;
			speedRight = right / period;
			speedLeft  = left  / period;
			float errRight = (refSpeedRight - speedRight);
			float errLeft  = (refSpeedLeft  - speedLeft );
			errIntRight += (errRight * period);
			errIntLeft  += (errLeft  * period);
			if ((refSpeedRight != 0.0f) || (refSpeedLeft != 0.0f)){
				float rightTorque =   kp * errRight + ki * errIntRight + refSpeedRight / MAX_SPEED * 1000;
				float leftTorque  = - kp * errLeft  - ki * errIntLeft  - refSpeedLeft  / MAX_SPEED * 1000;
				setMotor((int)rightTorque, (int)leftTorque);
			} else {
				setMotor(0, 0);
			}
		}
	}

	return 0;
}

/*!
 * @brief MegaRover�̃W���C�X�e�B�b�N�̏�Ԃ��擾����
 * x, y (0-1)
 * �Q�[���p�b�h�̃{�^��(��,��,�E,��,����,R3,L3,�ڸ�,��,�~,��,��,R1,L1,R2,L2)
 *
 * @param[in] x �W���C�X�e�B�b�N��x���̒l(-1�`1)
 * @param[in] y �W���C�X�e�B�b�N��y���̒l(-1�`1)
 * @param[in] b �W���C�X�e�B�b�N�̃{�^���̒l
 *
 * @return 0
 */
int megaRover::getJoyStick(float *x, float *y, int *b)
{
	if (comMutex == NULL) return -1;
	WaitForSingleObject(comMutex, INFINITE); 
	CWRC_ReadMemMap(GamePadButton, 10);
	CWRC_ReadExecute();

	*b = GetMem_SWord(GamePadButton);
	*x = (float)GetMem_SWord(GamePadLJoyUD)/128.0f;
	*y = (float)GetMem_SWord(GamePadRJoyLR)/128.0f;
	ReleaseMutex(comMutex);

	return 0;
}


int megaRover::getReferenceSpeed(float *right, float *left){
	*right = refSpeedRight;
	*left  = refSpeedLeft ;

	return 0;
}


/*!
 * @brief �I�h���g���̊p�x�̐ݒ�
 * �W���C���I�h���g���̂��߂ɕ��ʂ�ݒ肷��(rad)
 *
 * @param[in] angle �I�h���g���̊p�x(rad)
 *
 * @return 0
 */
int megaRover::setOdometoryAngle(float angle)
{
	if (mutex == NULL) return -1;
	WaitForSingleObject(mutex, INFINITE);
	odoThe = angle;
	if (odoThe >  M_PI) odoThe -= 2.0f*M_PI;
	if (odoThe < -M_PI) odoThe += 2.0f*M_PI;
	ReleaseMutex(mutex);
	
	return 0;
}

/*!
 * @brief �I�h���g���̒l���N���A����D
 *
 * @return 0
 */
int megaRover::clearOdometory()
{
	if (mutex == NULL) return -1;
	WaitForSingleObject(mutex, INFINITE);
	odoX = odoY = odoThe = 0;
	ReleaseMutex(mutex);		

	return 0;
}

