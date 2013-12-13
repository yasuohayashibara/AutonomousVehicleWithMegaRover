// IMU�ɓƎ��d�l�̊�ƃv���g�R���ŒʐM���Ă���

#include "stdafx.h"
#include "imu.h"

/*!
 * @class imu
 * @brief IMU���g�����߂̃N���X
 * @author Y.Hayashibara
 */

/*!
 * @brief �R���X�g���N�^
 */
imu::imu()
{
}

/*!
 * @brief �f�X�g���N�^
 */
imu::~imu()
{
}

/*!
 * @brief ������
 *
 * @param[in] com_port �ʐM�|�[�g�̔ԍ�(1-)
 *
 * @return true:�����Cfalse:���s
 */
int imu::Init(int com_port)
{
	int res = comm.Open(com_port, 115200);
	comm.Send("0");

	return res;
}

/*!
 * @brief �I������
 *
 * @return 0
 */
int imu::Close()
{
	comm.Close();
	return 0;
}

/*!
 * @brief IMU�̃��Z�b�g
 * IMU�̃I�t�Z�b�g�̃��Z�b�g (7�b�Ԓ�~)
 *
 * @return 0
 */
int imu::Reset()
{
	comm.Send("a");
	Sleep(7000);			// reset��6.6�b������
	comm.ClearRecvBuf();	// ��M�p�o�b�t�@���N���A

	return 0;
}

/*!
 * @brief �p�x�̎擾�̊J�n
 *
 * @return 0
 */
int imu::GetAngleStart()
{
	comm.Send("e");

	return 0;
}

/*!
 * @brief �p�x�̎擾
 *
 * @param[in] x x�������̊p�x(rad)
 * @param[in] y y�������̊p�x(rad)
 * @param[in] z z�������̊p�x(rad)
 *
 * @return 0
 */
int imu::GetAngle(float *x, float *y, float *z)
{
	const float COEF = -0.00836181640625f;
	const int max_no = 100;
	static char buf[max_no];
	char w[10], *e;
	int x1, y1, z1;

	if (comm.Recv(buf, max_no) <= 0) return -1;
	strncpy(w, buf  , 4);
	x1 = strtol(w, &e, 0x10);
	if (x1 >= 0x8000) x1 -= 0x10000;
	strncpy(w, buf+4, 4);
	y1 = strtol(w, &e, 0x10);
	if (y1 >= 0x8000) y1 -= 0x10000;
	strncpy(w, buf+8, 4);
	z1 = strtol(w, &e, 0x10);
	if (z1 >= 0x8000) z1 -= 0x10000;

	*x = COEF * x1;
	*y = COEF * y1;
	*z = COEF * z1;

	return 0;
}
