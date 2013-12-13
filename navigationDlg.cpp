/*!
 * @file  navigationDlg.cpp
 * @brief �_�C�A���O�N���X
 * @date 2013.10.31
 * @author Y.Hayashibara
 *
 * �S�̂𓝊�����ł���{�I�ȃN���X
 * OnInitialize()�ŏ�����
 * OnTimer()�Ŏ����I�ȏ���
 * OnClose()�ŏI������
 */

#include "stdafx.h"
#include <mmsystem.h>
#include <math.h>
#include "navigation.h"
#include "navigationDlg.h"
#include "logger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define	M_PI	3.14159f

#define USE_URG3D
#define USE_MEGA_ROVER
#define USE_IMU
#define USE_CAMERA

// �A�v���P�[�V�����̃o�[�W�������Ɏg���� CAboutDlg �_�C�A���O

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

// ����
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

/*!
 * @class CnavigationDlg
 * @brief �_�C�A���O�̃N���X
 * @author Y.Hayashibara
 */

// CnavigationDlg �_�C�A���O

CnavigationDlg::CnavigationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CnavigationDlg::IDD, pParent)
	, message(_T("")), is_record(0), is_play(0)
	, is_target_ip(0), coincidence(0)
	, m_check_loop(FALSE), reroute_mode0(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CnavigationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PAINT, navigationView);
	DDX_Text(pDX, IDC_EDIT_MESSAGE, message);
	DDX_Check(pDX, IDC_CHECK_LOOP, m_check_loop);
}

BEGIN_MESSAGE_MAP(CnavigationDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_RECORD, &CnavigationDlg::OnBnClickedButtonRecord)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &CnavigationDlg::OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_CHECK_LOOP, &CnavigationDlg::OnBnClickedCheckLoop)
END_MESSAGE_MAP()


// CnavigationDlg ���b�Z�[�W �n���h��

/*!
 * @brief ������
 *
 * @return TRUE
 */
BOOL CnavigationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// "�o�[�W�������..." ���j���[���V�X�e�� ���j���[�ɒǉ����܂��B

	// IDM_ABOUTBOX �́A�V�X�e�� �R�}���h�͈͓̔��ɂȂ���΂Ȃ�܂���B
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���̃_�C�A���O�̃A�C�R����ݒ肵�܂��B�A�v���P�[�V�����̃��C�� �E�B���h�E���_�C�A���O�łȂ��ꍇ�A
	//  Framework �́A���̐ݒ�������I�ɍs���܂��B
	SetIcon(m_hIcon, TRUE);			// �傫���A�C�R���̐ݒ�
	SetIcon(m_hIcon, FALSE);		// �������A�C�R���̐ݒ�

	ShowWindow(SW_SHOW);

#ifdef _DEBUG
	char s[100];
	time_t timer = time(NULL);
	struct tm *date = localtime(&timer);
	sprintf(s, "log%04d%02d%02d%02d%02d.txt", date->tm_year+1900, date->tm_mon+1, date->tm_mday, date->tm_hour, date->tm_min);

	logger::Init(s);
	LOG("START\n");
#endif

	timeBeginPeriod(1);
#ifdef USE_URG3D
	urg3d.Init();
	urg3d.SetTiltAngle(0, 45, 1);
#endif
#ifdef USE_MEGA_ROVER 
	if (mega_rover.init()) AfxMessageBox("Connect Mega Rover");
	mega_rover.servoOn(30);			// Gain��ݒ�
	mega_rover.setSpeedControlMode(1);
	mega_rover.setDelta(20,20);		// �������̒��x
#endif
#ifdef USE_IMU
	IMU.Init(IMU_COM_PORT);
#endif
#ifdef USE_CAMERA
	ip.init();
#endif
	navigation.Init();
	obs_avoid.Init();
	detect_target.Init();

	SetTimer(1, 100, NULL);
	
	return TRUE;					// �t�H�[�J�X���R���g���[���ɐݒ肵���ꍇ�������ATRUE ��Ԃ��܂��B
}

void CnavigationDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �_�C�A���O�ɍŏ����{�^����ǉ�����ꍇ�A�A�C�R����`�悷�邽�߂�
//  ���̃R�[�h���K�v�ł��B�h�L�������g/�r���[ ���f�����g�� MFC �A�v���P�[�V�����̏ꍇ�A
//  ����́AFramework �ɂ���Ď����I�ɐݒ肳��܂��B

void CnavigationDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);			// �`��̃f�o�C�X �R���e�L�X�g

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// �N���C�A���g�̎l�p�`�̈���̒���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �A�C�R���̕`��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// ���[�U�[���ŏ��������E�B���h�E���h���b�O���Ă���Ƃ��ɕ\������J�[�\�����擾���邽�߂ɁA
//  �V�X�e�������̊֐����Ăяo���܂��B
HCURSOR CnavigationDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/*!
 * @brief �E�B���h�E�����Ƃ��̃n���h��
 */
void CnavigationDlg::OnClose()
{
#ifdef USE_URG3D
	urg3d.Close();
#endif
#ifdef USE_MEGA_ROVER
	mega_rover.close();
#endif
#ifdef USE_IMU
	IMU.Close();
#endif
	navigation.Close();
	obs_avoid.Close();
	detect_target.Close();

	timeEndPeriod(1);

#ifdef _DEBUG
	logger::Close();
#endif
	CDialog::OnClose();
}

/*!
 * @brief ������(100ms)�ŌĂяo�����n���h��
 * ���܂萳�m�Ȏ����ł͌Ăяo����Ȃ�
 */
void CnavigationDlg::OnTimer(UINT_PTR nIDEvent)
{
	// �ʒu�̕␳�ɗp����f�[�^�͈̔́i���{�b�g���W�j
	static const int search_x0 =      0, search_x1 = 14000;			//! �O������̒T���͈�(mm)	�yToDo: �����ƒT���͈͂��L���������ǂ��̂ł͂Ȃ����H�z
	static const int search_y0 = -14000, search_y1 = 14000;			//! ���E�����̒T���͈�(mm)

	static const int search_z0     = 1800, search_z1     = 1900;	//! �㉺�����̒T���͈�(mm) �����p
//	static const int search_z0     = 1900, search_z1     = 2000;	//! �㉺�����̒T���͈�(mm) ���O�p
	static const int search_obs_z0 =    0, search_obs_z1 = 1000;	//! ��Q����T���㉺�����̒T���͈�(mm)�@URG�̍�������@�yToDo: ��܂ŒT�������ق����ǂ��̂ł͂Ȃ����H�z
	static const int search_tar_z0 =  200, search_tar_z1 = 400;		//! �^�[�Q�b�g��T���㉺�����̒T���͈�(mm)�@URG�̍������
	static const int intensity_thre = 7000;							//! ���ˋ��x�̂������l(�P�ʂȂ�)

	static int is_first = 1;
	float odoX = 0, odoY = 0, odoThe = 0;							//! �I�h���g���̈ʒu�Ɗp�x(m, rad)
	float estX = 0, estY = 0, estThe = 0;							//! ���肵���ʒu�Ɗp�x(m, rad)
	float joyX, joyY;
	int button;
#ifdef MEGA_ROVER_1_1
	static const float f_gain = 0.5f;								//! �W���C�X�e�B�b�N��O��ɍő���|�������̑��x�ڕW(m/s)
	static const float t_gain = 0.5f;								//! �W���C�X�e�B�b�N�����E�ɍő���|�������Ƀz�C�[�����t�ɉ�]���鑬�x�ڕW(m/s)
#else
	static const float f_gain = 0.3f;								//! �W���C�X�e�B�b�N��O��ɍő���|�������̑��x�ڕW(m/s)
	static const float t_gain = 1.0f;								//! �W���C�X�e�B�b�N�����E�ɍő���|�������Ƀz�C�[�����t�ɉ�]���鑬�x�ڕW(m/s)
#endif
	float target_ip_cf = 0.0f;

#ifdef USE_CAMERA
	is_target_ip = ip.checkTarget(& target_ip_cf);					//! �T���Ώۂ��ǂ����̃`�F�b�N(true:�T���ΏہCfalse:�T���ΏۂłȂ��j
#endif

#ifdef USE_MEGA_ROVER
	// �p�[�e�B�N���̕\��
	static const int MAX_PARTICLE_NUM = 1000;						//! �ő�1000�\��
	static struct particle_T particle[MAX_PARTICLE_NUM];			//! �p�[�e�B�N���̔z��
	int particle_num;												//! ���ݎg�p���Ă���p�[�e�B�N��

	float rightSpeed, leftSpeed;
	mega_rover.getSpeed(&rightSpeed, &leftSpeed);
	LOG("rightSpeed:%f, leftSpeed%f\n", rightSpeed, leftSpeed);
	mega_rover.getOdometory(&odoX, &odoY, &odoThe, 0);			// �I�h���g�����擾
	LOG("odometory:(%f,%f,%f)\n", odoX, odoY, odoThe);
	estX = odoX, estY = odoY, estThe = odoThe;					// �p�[�e�B�N���t�C���^���g�p���Ă��Ȃ��Ƃ��́C����ʒu�ƃI�h���g���͈ꏏ
	mega_rover.getJoyStick(&joyX, &joyY, &button);				// �W���C�X�e�B�b�N�̒l���擾
	if (fabs(joyX) < 0.1f) joyX = 0.0f;
	if (fabs(joyY) < 0.1f) joyY = 0.0f;
	LOG("joystick:(%f,%f),button:%d\n", joyX, joyY, button);
	{
		char temp[256];
		sprintf(temp, "joystick %d", button);
		message = temp;
		UpdateData(FALSE);
	}

	if (!is_record){											// �������[�h�łȂ�����
		if (button & (DOWN_BUTTON | RIGHT_BUTTON | LEFT_BUTTON)){	// �{�^���P����������
			is_play = 0;										// ��~
			mega_rover.setMotor(0,0);
		} else if (button & UP_BUTTON){							// �{�^���Q����������
			if (!navigation.isPlayMode()) OnBnClickedButtonPlay();
			is_play = 1;										// �Đ�
		}
	}
	if (navigation.setOdometory(odoX, odoY, odoThe)){			// �i�r�Q�[�V�����ɃI�h���g�������
		LOG("goal\n");
		if (m_check_loop){										// ���񂷂�ꍇ
			urg3d.ClearData();
			navigation.Init();									// �e�p�����[�^�����Z�b�g����D
			obs_avoid.Init();
			mega_rover.setMotor(0,0);							// ���[�^��~
			mega_rover.clearOdometory();						// �I�h���g���̃N���A
		} else {												// ���񂵂Ȃ��ꍇ�́C�Đ����[�h���~���Ē�~����D
			is_play = 0;
			mega_rover.setMotor(0,0);
		}
	};
	LOG("is_play:%d, is_record:%d\n", is_play, is_record);
	if (is_play){												// �������s
		float front, radius;
		float tarX, tarY, tarThe, period;
		
		navigation.getEstimatedPosition(&estX, &estY, &estThe);	// ����ʒu�̎擾
		LOG("estimatePosition:(%f,%f,%f)\n", estX, estY, estThe);
		navigation.getTargetPosition(&tarX, &tarY, &tarThe, &period);
																// �ڕW�ʒu�̎擾
		LOG("targetPosition:(%f,%f,%f)\n", tarX, tarY, tarThe);
		float slowDownFactor = obs_avoid.getSlowDownFactor();	// ��Q�����������Ƃ��̌�������䗦���擾
		navigation.setNeedStop(obs_avoid.isNeedStop());
		
		if (reroute_mode0 && (!navigation.isRerouteMode())){
			obs_avoid.finishReroute();
			reroute_mode0 = 0;
		}

		if (obs_avoid.isReroute()){
			float forward, rotate;
			if (!reroute_mode0){
				navigation.setRerouteMode(obs_avoid.getRerouteDirection());
				reroute_mode0 = 1;
			}
			navigation.getSpeed(&forward, &rotate);
			mega_rover.setSpeed(forward * slowDownFactor, rotate);
			LOG("reroute_mode, forward;%f, rotate:%f, slowDownFactor:%f\n", forward, rotate, slowDownFactor);
		} else if (navigation.isSearchMode()){
			float forward, rotate;
			navigation.getSpeed(&forward, &rotate);
			mega_rover.setSpeed(forward * slowDownFactor, rotate);
			LOG("search_mode, forward;%f, rotate:%f, slowDownFactor:%f\n", forward, rotate, slowDownFactor);
		} else {
			if (slowDownFactor < 1.0f) PlaySound("obstacle.wav", NULL, SND_FILENAME | SND_ASYNC | SND_NOSTOP);
			navigation.getTargetArcSpeed(&front, &radius);		// �ڕW�Ɉړ����鑬�x�Ɣ��a
			mega_rover.setArcSpeed(front * slowDownFactor, radius);
			LOG("target_arc_speed, front;%f, radius:%f, slowDownFactor:%f\n", front, radius, slowDownFactor);
		}

		navigationView.setOdometory(estX, estY, estThe);		// ����ʒu�̓���
		navigationView.setTargetPos(tarX, tarY, tarThe);		// �ڕW�ʒu�̓���
		
		// �p�[�e�B�N���̎擾�ƕ\��
		navigation.getParticle(particle, &particle_num, MAX_PARTICLE_NUM);
		LOG("\n");
		for(int i = 0;i < particle_num; i ++){
			LOG_WITHOUT_TIME("particle:(%f,%f,%f), eval:%d\n", particle[i].x, particle[i].y, particle[i].the, particle[i].eval);
		}
		navigationView.setParticle(particle, particle_num);		// �p�[�e�B�N���̕\��
		
		// ��v�x�̎擾�ƕ\��
		navigation.getCoincidence(&coincidence);
		navigationView.setCoincidence(coincidence);
	} else {													// �Đ����[�h�łȂ��ꍇ
		mega_rover.setSpeed(f_gain * joyY, t_gain * joyX);
		navigationView.setOdometory(odoX, odoY, odoThe);
	}
	navigationView.setStep(navigation.getStep());				// waypoint�̐��̕\��

#endif

#ifdef USE_URG3D
	// URG3D�����M����f�[�^
	static const int MAX_DATA = 10000;
	static pos p[MAX_DATA];
	int num = 0;

	// URG3D�����M�����Q���̃f�[�^
	static const int MAX_OBS_DATA = 10000;
	static pos op[MAX_OBS_DATA];
	int obs_num = 0;

	// URG3D�����M����^�[�Q�b�g�̃f�[�^
	static const int MAX_TAR_DATA = 10000;
	static pos_inten tp[MAX_TAR_DATA];
	int tar_num = 0;

	// �O���[�o�����W�ł̃^�[�Q�b�g�̃f�[�^
	static pos_inten global_tp[MAX_TAR_DATA];

	// �\���p�̃f�[�^�i�ǉ����Ă����j
	static const int MAX_DRAW_POS = 10000;
	static pos drawPos[MAX_DRAW_POS];
	static int draw_no = 0, is_max = 0;

	// ���ˋ��x�\���p�̃f�[�^�i�ǉ����Ă����j
	static const int MAX_DRAW_INTEN_POS = 10000;
	static pos_inten drawIntenPos[MAX_DRAW_INTEN_POS];
	static int inten_draw_no = 0, is_max_inten = 0;

	// �T���Ώۂ̌��̃f�[�^
	static const int MAX_SLATE_POINT = 100;
	static pos_slate slatePoint[MAX_SLATE_POINT];
	static int slate_num = 0;

	// �i�r�Q�[�V�����ɑ��M����f�[�^
	static const int MAX_NAVI_POS = 10000;
	static pos naviPos[MAX_NAVI_POS];
	int navi_num = 0;

	// �Q�ƃf�[�^
	static const int MAX_REF_DATA = 10000;
	static pos rp[MAX_REF_DATA];
	int ref_num = 100;
	
	// URG3D����f�[�^���擾
	urg3d.Get3SelectedData(search_z0, search_z1, p, &num, MAX_DATA,
		search_obs_z0, search_obs_z1, op, &obs_num, MAX_OBS_DATA,
		search_tar_z0, search_tar_z1, tp, &tar_num, MAX_TAR_DATA, intensity_thre);
	
	// world���W�n�ɕϊ� (search_x,y�ŗ̈�𐧌�)
	LOG("\n");
	for(int i = 0; i < num; i ++){
		if ((p[i].x < search_x0)||(p[i].x > search_x1)||
			(p[i].y < search_y0)||(p[i].y > search_y1)) continue;
		int x = (int)(p[i].x * cos(odoThe) - p[i].y * sin(odoThe) + odoX * 1000.0f);		// �i�r�Q�[�V�����ɂ̓I�h���g���x�[�X�̘A���I�ȃf�[�^�𑗂�
		int y = (int)(p[i].x * sin(odoThe) + p[i].y * cos(odoThe) + odoY * 1000.0f);
		naviPos[navi_num].x = x, naviPos[navi_num].y = y, naviPos[navi_num].z = p[i].z;
		navi_num ++;
		
		x = (int)(p[i].x * cos(estThe) - p[i].y * sin(estThe) + estX * 1000.0f);			// �\���ɂ͐���ʒu����Ƃ����l��p����
		y = (int)(p[i].x * sin(estThe) + p[i].y * cos(estThe) + estY * 1000.0f);			// ���肵�Ă��Ȃ��Ƃ��́C�I�h���g���Ɠ����ɂ���D
		LOG_WITHOUT_TIME("urg_global_pos:(%d,%d,%d)\n", x, y, p[i].z);
		drawPos[draw_no].x = x, drawPos[draw_no].y = y, drawPos[draw_no].z = p[i].z;
		draw_no ++;
		if (draw_no >= MAX_DRAW_POS) is_max = 1, draw_no = 0;
	}

	navigation.setData(naviPos, navi_num);													// �i�r�Q�[�V�����Ƀf�[�^��o�^

	if (is_max) navigationView.setData(drawPos, MAX_DRAW_POS);								// �f�[�^�̕`��
	else        navigationView.setData(drawPos, draw_no     );								// �f�[�^�̕`��

	// world���W�n�ɕϊ� (search_x,y�ŗ̈�𐧌�)
	LOG("\n");
	for(int i = 0; i < tar_num; i ++){		
		int x = (int)(tp[i].pos.x * cos(estThe) - tp[i].pos.y * sin(estThe) + estX * 1000.0f);	// �\���ɂ͐���ʒu����Ƃ����l��p����
		int y = (int)(tp[i].pos.x * sin(estThe) + tp[i].pos.y * cos(estThe) + estY * 1000.0f);	// ���肵�Ă��Ȃ��Ƃ��́C�I�h���g���Ɠ����ɂ���D
		LOG_WITHOUT_TIME("intensity_global_pos:(%d,%d,%d),intensity:%d\n", x, y, tp[i].pos.z, tp[i].intensity);
		global_tp[i].pos.x = x, global_tp[i].pos.y = y, global_tp[i].pos.z = tp[i].pos.z;
		global_tp[i].intensity = tp[i].intensity;

		drawIntenPos[inten_draw_no].pos.x = x, drawIntenPos[inten_draw_no].pos.y = y, drawIntenPos[inten_draw_no].pos.z = tp[i].pos.z;
		drawIntenPos[inten_draw_no].intensity = tp[i].intensity;
		inten_draw_no ++;
		if (inten_draw_no >= MAX_DRAW_INTEN_POS) is_max_inten = 1, inten_draw_no = 0;
	}
	detect_target.addIntensityData(global_tp, tar_num);										// �������l�ȏ�̔��ˋ��x�f�[�^���^�[�Q�b�g���o�̃N���X�ɐݒ�
	detect_target.getTargetPos(slatePoint, &slate_num);										// �T���Ώێ҂̌��_���擾
	{
		const float SEARCH_RADIUS = 5.0f;
		pos search_pos, self_pos;
		self_pos.x = (int)(estX * 1000), self_pos.y = (int)(estY * 1000), self_pos.z = 0;

		if (detect_target.getSearchPos(&search_pos, self_pos, SEARCH_RADIUS)){
			if (navigation.distaceFromPreviousSearchPoint() > SEARCH_RADIUS) navigation.setSearchPoint(search_pos);			// ��蓹������悤��navi�N���X�Ɉʒu��ݒ�
		}
	}
	navigationView.setSlatePoint(slatePoint, slate_num);									// �T���Ώێ҂̌��_��`�悷��悤�ɐݒ�
	LOG("\n");
	for(int i = 0; i < slate_num; i ++){		
		LOG_WITHOUT_TIME("slate_point:(%d,%d,%d),probability:%f\n", slatePoint[i].pos.x, slatePoint[i].pos.y, slatePoint[i].pos.z, slatePoint[i].probability);
	}

	if (is_max_inten) navigationView.setIntensityData(drawIntenPos, MAX_DRAW_INTEN_POS);	// �f�[�^�̕`��
	else			  navigationView.setIntensityData(drawIntenPos, inten_draw_no     );	// �f�[�^�̕`��

	navigation.getRefData(rp, &ref_num, MAX_REF_DATA);										// ���t�@�����X�f�[�^�̕\��
	navigationView.setRefData(rp, ref_num);

	obs_avoid.setData(op, obs_num);															// ��Q���f�[�^�̑��
#endif

#ifdef USE_IMU
	float xt = 0, yt = 0, zt = 0;

	IMU.GetAngleStart();
	if (IMU.GetAngle(&xt, &yt, &zt) == 0){
		mega_rover.setOdometoryAngle(zt * M_PI / 180.0f);

/*		{
			char temp[256];
			sprintf(temp, "angle xt:%f, yt:%f, zt:%f", xt, yt, zt);
			message = temp;
			UpdateData(FALSE);
		}*/
	}
#endif
	is_first = 0;
	CDialog::OnTimer(nIDEvent);
}

/*!
 * @brief [Record]�{�^�������������̃n���h��
 */
void CnavigationDlg::OnBnClickedButtonRecord()
{
	if (!is_play)   is_record ^= 1;
	navigation.setTargetFilename();
	navigation.setRecordMode(is_record);
	navigationView.setStatus(is_record, is_play);
}

/*!
 * @brief [Play]�{�^�������������̃n���h��
 */
void CnavigationDlg::OnBnClickedButtonPlay()
{
	if (!is_record) is_play ^= 1;
	navigation.setTargetFilename("navi.csv");
	navigation.setPlayMode(is_play);
	navigationView.setStatus(is_record, is_play);
}

/*!
 * @brief [loop]�̃`�F�b�N�{�b�N�X�����������̃n���h��
 */
void CnavigationDlg::OnBnClickedCheckLoop()
{
	UpdateData(true);
}
