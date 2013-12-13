#pragma once

// ���ʂ̃f�[�^�^���`

/*!
 * @struct pos_T
 * @brief �ʒu�f�[�^
 */
struct pos_T{
	int x;				//!< x���W(mm)
	int y;				//!< y���W(mm)
	int z;				//!< z���W(mm)
};
typedef struct pos_T pos;

/*!
 * @struct pos_inten_T
 * @brief ���ˋ��x�t���ʒu�f�[�^
 */
struct pos_inten_T{
	pos pos;			// �ʒu�f�[�^(mm)
	int intensity;		// ���ˋ��x�i�P�ʂȂ��j
};
typedef struct pos_inten_T pos_inten;

/*!
 * @struct pos_slate_T
 * @brief �T���Ώۂ̌��̈ʒu�f�[�^
 * �m��(0-1)�t��
 */
struct pos_slate_T{
	pos pos;			// �ʒu�f�[�^(mm)
	float probability;	// �m��(0.0-1.0)
};
typedef struct pos_slate_T pos_slate;

/*!
 * @struct particle_T
 * @brief �p�[�e�B�N���̃f�[�^
 */
struct particle_T{
	float x;			//!< x���W(m)
	float y;			//!< y���W(m)
	float the;			//!< �p�x(rad)
	int eval;			//!< �]��(0-)
};

/*!
 * @struct odometory
 * @brief �I�h���g���̃f�[�^
 */
struct odometory{
	float x;			//!< x���W(m)
	float y;			//!< y���W(m)
	float the;			//!< �p�x(rad)
};
