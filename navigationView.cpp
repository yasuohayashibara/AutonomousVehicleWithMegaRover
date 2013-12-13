// navigationView.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "navigation.h"
#include "navigationView.h"
#include <math.h>
#include <mmsystem.h>
#include <time.h>

#define	M_PI	3.14159f

// CnavigationView

IMPLEMENT_DYNAMIC(CnavigationView, CStatic)

/*!
 * @class CnavigationView
 * @brief ���Ȉʒu���Q���f�[�^�Ɋւ��ĕ\�����s���N���X
 * @author Y.Hayashibara
 */

/*!
 * @brief �R���X�g���N�^
 */
CnavigationView::CnavigationView():
data_num(0), ref_data_num(0), odo_num(0), ratio(0.02f), center_x(0), center_y(0),
is_target_view(0), particle_num(0), time0(0), step(0), coincidence(0),
is_record(), is_play(0)
{
}

/*!
 * @brief �f�X�g���N�^
 */
CnavigationView::~CnavigationView()
{
}

// CnavigationView ���b�Z�[�W �n���h��
BEGIN_MESSAGE_MAP(CnavigationView, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

/*!
 * @brief �ĕ`��
 */
void CnavigationView::OnPaint()
{
	CPaintDC mdc(this); // device context for painting

	// View�̃T�C�Y���擾
	CRect rc;
	GetClientRect(&rc);
	mdc.Rectangle(&rc);
	disp_x = rc.Width(), disp_y = rc.Height();
	center_x = disp_x / 2, center_y = disp_y / 2;

	int p;

	// �`��̈�̐ݒ�
	CDC dc;
	dc.CreateCompatibleDC(&mdc);
	CBitmap memBmp;
	memBmp.CreateCompatibleBitmap(&mdc,disp_x, disp_y);
	CBitmap *pOldBmp = dc.SelectObject(&memBmp);
	dc.Rectangle(0, 0, disp_x, disp_y);

	// �t�H���g�̐ݒ�
	HFONT hFont = CreateFont(60, //�t�H���g����
		0, //������
		0, //�e�L�X�g�̊p�x
		0, //�x�[�X���C���Ƃ����Ƃ̊p�x
		FW_DONTCARE, //�t�H���g�̏d���i�����j
		FALSE, //�C�^���b�N��
		FALSE, //�A���_�[���C��
		FALSE, //�ł�������
		DEFAULT_CHARSET, //�����Z�b�g
		CLIP_DEFAULT_PRECIS, //�o�͐��x
		CLIP_DEFAULT_PRECIS,//�N���b�s���O���x
		CLEARTYPE_QUALITY, //�o�͕i��
		VARIABLE_PITCH,//�s�b�`�ƃt�@�~���[
		_T("Impact"));
	HFONT prevFont = (HFONT)SelectObject(dc, hFont);

	HPEN hPen = (HPEN)CreatePen(PS_SOLID, 1, RGB(0,0,0));
	HPEN prevPen = (HPEN)SelectObject(dc, hPen);

	// �����̕\��
	rc.left = 10, rc.top = 10;
	dc.DrawText(_T("MEGA ROVER"), -1, &rc, NULL);

	// �����̕\��
	{
		char buf[20];
		sprintf(buf, "STEP: %04d\n",step);
		rc.left = 400, rc.top = 10;
		dc.DrawText(_T(buf), -1, &rc, NULL);
	}

	// �����̕\��
	{
		if (is_record){
			rc.left = 10, rc.top = 400;
			dc.DrawText(_T(">> RECORD"), -1, &rc, NULL);
		} else if (is_play){
			rc.left = 10, rc.top = 400;
			dc.DrawText(_T("PLAY >>"), -1, &rc, NULL);
		}
	}

	// ���[���h���W�n�ł̒��S�����߂�
	int x0 = 0, y0 = 0;
	if (odo_num > 0){
		x0 = (int)(odo[0].x * 1000);
		y0 = (int)(odo[0].y * 1000);
	}

	// �}�b�v�̕\��
	{
		CString num;
		memBmp.GetBitmapBits(disp_x*disp_y*4,rgb_thre);
		dc.SetROP2(R2_COPYPEN);
		struct bitmap_rgb_T rgb;

		rgb.red = rgb.green = rgb.blue = 0;
		
		int x, y;
		for(int i = 0; i < data_num; i ++){
			translatePos(data_pos[i].x, data_pos[i].y, x0, y0, &x, &y);
			if ((x >= 0)&&(x < disp_x)&&(y >= 0)&&(y < disp_y)){
				p = x + y * disp_x;
				rgb_thre[p] = rgb;
			}
		}
		// �Q�ƃf�[�^������ꍇ�́C�ŕ\��
		rgb.red = rgb.green = 0, rgb.blue = 255;
		for(int i = 0; i < ref_data_num; i ++){
			translatePos(ref_pos[i].x, ref_pos[i].y, x0, y0, &x, &y);
			if ((x >= 0)&&(x < disp_x)&&(y >= 0)&&(y < disp_y)){
				p = x + y * disp_x;
				rgb_thre[p] = rgb;
			}
		}
		// ���ˋ��x�f�[�^������ꍇ�́C�Ԃŕ\���@�i���x�ɉ����Ĕ�����Ԃɕω��j
		for(int i = 0; i < inten_data_num; i ++){
			int red = (int)min(max((float)inten_pos[i].intensity / 4000 * 255, 0), 255);
			rgb.red = 255, rgb.green = rgb.blue = 255 - red;
			translatePos(inten_pos[i].pos.x, inten_pos[i].pos.y, x0, y0, &x, &y);
			if ((x >= 0)&&(x < disp_x)&&(y >= 0)&&(y < disp_y)){
				p = x + y * disp_x;
				rgb_thre[p] = rgb;
			}
		}
		// ��v�x�̃O���t��`��
		for(int i = 0; i < (coincidence * 200); i ++){
			y = 400 - i;
			if (y < 0) break;
			rgb.red = 255 - i, rgb.green = i, rgb.blue = 0;
			for(x = 500; x < 550; x ++){
				p = x + y * disp_x;
				rgb_thre[p] = rgb;
			}
		}

		memBmp.SetBitmapBits(disp_x*disp_y*4,rgb_thre);
		dc.SetBkColor(RGB(0xff,0xff,0xff));
	}
	
	// �T���Ώی��_��Ԃŕ\���@�i�m���ɉ����Ĕ��a���ω��j
	{
		static const int MAX_RADIUS = 10;
		int x, y, radius;
		DeleteObject(hPen);
		hPen = (HPEN)CreatePen(PS_SOLID, 1, RGB(255,0,0));		// �ԐF�̃y����I��
		SelectObject(dc, hPen );
		for(int i = 0; i < slate_data_num; i ++){
			radius = (int)(slate_pos[i].probability * MAX_RADIUS);
			translatePos(slate_pos[i].pos.x, slate_pos[i].pos.y, x0, y0, &x, &y);
			dc.Arc(x - radius, y - radius, x + radius, y + radius, x - radius, y, x - radius, y);
		}
		SelectObject(dc, prevPen );
	}

	// �p�[�e�B�N���̕\��
	{
		static const int RADIUS = 3, ANG_LEN = 5;
		float angle;
		int x, y, px, py, x1, y1;
		DeleteObject(hPen);
		hPen = (HPEN)CreatePen(PS_SOLID, 1, RGB(128,128,128));		// �D�F�̃y����I��
		SelectObject(dc, hPen );
		for(int i = 0;i < particle_num; i ++){
			px = (int)(particle[i].x * 1000);
			py = (int)(particle[i].y * 1000);
			translatePos(px, py, x0, y0, &x, &y);
			dc.Ellipse(x - RADIUS, y - RADIUS, x + RADIUS, y + RADIUS);
			angle = particle[i].the;
			x1 = x + (int)(ANG_LEN * cos(angle));
			y1 = y - (int)(ANG_LEN * sin(angle));
			dc.MoveTo(x, y); dc.LineTo(x1, y1);
		}
		SelectObject(dc, prevPen );
	}

	// �I�h���g���̕\��
	if (odo_num > 0) {
		static const int RADIUS = 10, ANG_LEN = 20;
		int ox = (int)(odo[0].x * 1000), oy = (int)(odo[0].y * 1000);
		int x, y, x1, y1;
		float angle = odo[0].the;

		// ���݂̈ʒu�̕`��
		translatePos(ox, oy, x0, y0, &x, &y);
		dc.Ellipse(x - RADIUS, y - RADIUS, x + RADIUS, y + RADIUS);
		x1 = x + (int)(ANG_LEN * cos(angle));
		y1 = y - (int)(ANG_LEN * sin(angle));
		dc.MoveTo(x, y); dc.LineTo(x1, y1);

		// �����̕`��
		dc.MoveTo(x, y);
		for(int i = 1; i < odo_num;i ++){
			ox = (int)(odo[i].x * 1000);
			oy = (int)(odo[i].y * 1000);
			translatePos(ox, oy, x0, y0, &x, &y);
			DeleteObject(hPen);
			hPen = (HPEN)CreatePen(PS_SOLID, 1, RGB(0, 255 * (odo_num - i)/odo_num, 0));
			SelectObject(dc, hPen);
			dc.LineTo(x, y);
		}
		SelectObject(dc, prevPen);
	}

	// �S�[���̕`��
	if (is_target_view){
		static const int RADIUS = 5, ANG_LEN = 40;
		int tx = (int)(tarX * 1000), ty = (int)(tarY * 1000);
		int x, y, dx, dy;

		translatePos(tx, ty, x0, y0, &x, &y);
		DeleteObject(hPen);
		hPen = (HPEN)CreatePen(PS_SOLID, 1, RGB(255,0,0));		// �ԃy����I��
		SelectObject(dc, hPen );
		dc.Ellipse(x - RADIUS, y - RADIUS, x + RADIUS, y + RADIUS);
		dx =   (int)(ANG_LEN * cos(tarThe + M_PI / 2));
		dy = - (int)(ANG_LEN * sin(tarThe + M_PI / 2));
		dc.MoveTo(x - dx, y - dy); dc.LineTo(x + dx, y + dy);
		SelectObject(dc, prevPen );
	}
	
	SelectObject(dc, prevFont);
	DeleteObject(hFont);
	DeleteObject(hPen);

	// ��ʂ̐؂�ւ�
	mdc.BitBlt(0,0,disp_x,disp_y,&dc,0,0,SRCCOPY);
	dc.SelectObject(pOldBmp);
	memBmp.DeleteObject();
	dc.DeleteDC();
}

/*!
 * @brief ��Q���̈ʒu�f�[�^��ݒ�
 *
 * @param[in] p ��Q���̈ʒu�f�[�^
 * @param[in] num ��Q���̈ʒu�f�[�^�̐�
 *
 * @return �\�������Q���̈ʒu�f�[�^�̐�
 */
int CnavigationView::setData(pos *p, int num)
{
	data_num = min(num, MAX_DATA_NUM);
	pos *r = data_pos;
	for(int i = 0; i < data_num; i ++){
		*r ++ = *p ++;
	}
	InvalidateRect(NULL);

	return data_num;
}

/*!
 * @brief �Q�Ƃ����Q���̈ʒu�f�[�^�̐ݒ�
 *
 * @param[in] p �Q�Ƃ����Q���̈ʒu�f�[�^
 * @param[in] num �Q�Ƃ����Q���̈ʒu�f�[�^�̐�
 *
 * @return �Q�Ƃ����Q���̈ʒu�f�[�^�̐�
 */
int CnavigationView::setRefData(pos *p, int num)
{
	ref_data_num = min(num, MAX_REF_DATA_NUM);
	pos *r = ref_pos;
	for(int i = 0; i < ref_data_num; i ++){
		*r ++ = *p ++;
	}
	InvalidateRect(NULL);

	return ref_data_num;
}

/*!
 * @brief ���ˋ��x�̃f�[�^�̐ݒ�
 *
 * @param[in] p ���ˋ��x�t���ʒu�f�[�^
 * @param[in] num �Q�Ƃ����Q���̈ʒu�f�[�^�̐�
 *
 * @return �Q�Ƃ����Q���̈ʒu�f�[�^�̐�
 */
int CnavigationView::setIntensityData(pos_inten *p, int num)
{
	inten_data_num = min(num, MAX_INTEN_DATA_NUM);
	pos_inten *r = inten_pos;
	for(int i = 0; i < inten_data_num; i ++){
		*r ++ = *p ++;
	}
	InvalidateRect(NULL);

	return inten_data_num;
}

/*!
 * @brief �T���Ώۂ̌��̐ݒ�
 *
 * @param[in] p �T���Ώۂ̌��̈ʒu�f�[�^
 * @param[in] num �T���Ώۂ̌��̈ʒu�f�[�^�̐�
 *
 * @return �Q�Ƃ����Q���̈ʒu�f�[�^�̐�
 */
int CnavigationView::setSlatePoint(pos_slate *p, int num)
{
	slate_data_num = min(num, MAX_SLATE_DATA_NUM);
	pos_slate *r = slate_pos;
	for(int i = 0; i < slate_data_num; i ++){
		*r ++ = *p ++;
	}
	InvalidateRect(NULL);

	return slate_data_num;
}


/*!
 * @brief �I�h���g���̐ݒ�
 *
 * @param[in] x   �I�h���g����x���W(m)
 * @param[in] y   �I�h���g����x���W(m)
 * @param[in] the �I�h���g���̊p�x(rad)
 *
 * @return �\������I�h���g���̐�
 */
int CnavigationView::setOdometory(float x, float y, float the)
{
	const float step_period = 1.0f;						// �o�H���L�^�������(s)
	long time = timeGetTime();							// step_period�ȏ�̊Ԋu���J���ĕۑ��i�E�F�C�|�C���g�ƂȂ�j

	odo[0].x = x, odo[0].y = y, odo[0].the = the;
	if ((time - time0) > (int)(step_period * 1000)){
		if (odo_num < MAX_ODO_NUM) odo_num ++;
		for(int i = (odo_num - 1); i > 0; i --){
			odo[i] = odo[i - 1];
		}
		time0 = time;
	}
	InvalidateRect(NULL);

	return odo_num;
}

/*!
 * @brief waypoint�̐ݒ�
 *
 * @param[in] x   waypoint��x���W(m)
 * @param[in] y   waypoint��y���W(m)
 * @param[in] the waypoint�̊p�x(rad)
 *
 * @return 0
 */
int CnavigationView::setTargetPos(float x, float y, float the)
{
	is_target_view = 1;
	tarX = x;
	tarY = y;
	tarThe = the;
	InvalidateRect(NULL);

	return 0;
}

/*!
 * @brief �p�[�e�B�N���̐ݒ�
 *
 * @param[in] p   �p�[�e�B�N���̃f�[�^
 * @param[in] num �p�[�e�B�N���̐�
 *
 * @return �\������p�[�e�B�N���̐�
 */
int CnavigationView::setParticle(struct particle_T *p, int num)
{
	particle_num = min(num, MAX_PARTICLE_NUM);
	struct particle_T *q = particle;
	for(int i = 0;i < particle_num; i ++){
		*q ++ = *p ++;
	}
	InvalidateRect(NULL);

	return data_num;
}

// ���[���h���W����f�B�Y�v���C�̍��W�ɕϊ� (�P�ʂ�mm)
/*!
 * @brief ���[���h���W�n����f�B�Y�v���C�̍��W�ւ̕ϊ�
 *
 * @param[in]  worldX  ���[���h���W�n��x���W(m)
 * @param[in]  worldY  ���[���h���W�n��y���W(m)
 * @param[in]  worldX0 �f�B�X�v���C�̒��S�ƂȂ郏�[���h���W�n��x���W(m)
 * @param[in]  worldY0 �f�B�X�v���C�̒��S�ƂȂ郏�[���h���W�n��y���W(m)
 * @param[out] dispX   �f�B�X�v���C��x���W(dot)
 * @param[out] dispY   �f�B�X�v���C��y���W(dot)
 *
 * @return 0
 */
int CnavigationView::translatePos(int worldX, int worldY, int worldX0, int worldY0, int *dispX, int *dispY)
{
	*dispX = (int)(ratio * (worldX - worldX0) + center_x);
	*dispY = (int)(ratio * (disp_y - (worldY - worldY0)) + center_y);
	
	return 0;
}

/*!
 * @brief waypoint�̐��̐ݒ�
 *
 * @param[in] step waypoint�̐�
 *
 * @return 0
 */
int CnavigationView::setStep(int step)
{
	this->step = step;

	return 0;
}

/*!
 * @brief ��v�x�̐ݒ�
 *
 * @param[in] coincidence ��v�x(0-1)
 *
 * @return 0
 */
int CnavigationView::setCoincidence(float coincidence)
{
	this->coincidence = coincidence;

	return 0;
}

/*!
 * @brief ���(�ۑ����[�h�C�Đ����[�h)�̐ݒ�
 *
 * @param[in] is_record �ۑ����[�h���ǂ����̃t���O(1:�ۑ����[�h�C0:�ۑ����[�h�ł͂Ȃ�)
 * @param[in] is_play   �Đ����[�h���ǂ����̃t���O(1:�Đ����[�h�C0:�Đ����[�h�ł͂Ȃ�)
 *
 * @return 0
 */
int CnavigationView::setStatus(int is_record, int is_play)
{
	this->is_record = is_record;
	this->is_play   = is_play  ;

	return 0;
}
