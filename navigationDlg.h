// navigationDlg.h : �w�b�_�[ �t�@�C��
//

#pragma once

#include "urg3D.h"
#include "megaRover.h"
#include "imu.h"
#include "navi.h"
#include "navigationview.h"
#include "obstacleAvoidance.h"
#include "imageProcessing.h"
#include "detectTarget.h"

// CnavigationDlg �_�C�A���O
class CnavigationDlg : public CDialog
{
// �R���X�g���N�V����
public:
	CnavigationDlg(CWnd* pParent = NULL);	// �W���R���X�g���N�^

// �_�C�A���O �f�[�^
	enum { IDD = IDD_NAVIGATION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �T�|�[�g

// ����
protected:
	HICON m_hIcon;

	// �������ꂽ�A���b�Z�[�W���蓖�Ċ֐�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	urg3D urg3d;
	megaRover mega_rover;
	imu IMU;
	navi navigation;
	obstacleAvoidance obs_avoid;
	imageProcessing ip;
	detectTarget detect_target;

	int is_record;
	int is_play;
	int is_target_ip;
	float coincidence;
	
	int reroute_mode0;

public:

		
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CnavigationView navigationView;
	CString message;
	afx_msg void OnBnClickedButtonRecord();
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedCheckLoop();
	BOOL m_check_loop;
};
