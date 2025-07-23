
// AssignmentDlg.h: 헤더 파일
//

#pragma once
#include <vector>
#include <gdiplus.h>
#define WM_MY_UPDATE_CIRCLE (WM_USER + 1)

// CAssignmentDlg 대화 상자
class CAssignmentDlg : public CDialogEx
{
// 생성입니다.
public:
	CAssignmentDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ASSIGNMENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedBtnReset();
	afx_msg void OnBnClickedBtnRandomMove();
	afx_msg LRESULT OnMyUpdateCircle(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy(); // GDI+ 종료를 위해 추가
	DECLARE_MESSAGE_MAP()

private:
	std::vector<CPoint> m_points;      // 클릭 지점들을 저장할 공간
	int m_clickCount;                  // 클릭 횟수 
	int m_nRadius;                     // 클릭 지점 원의 반지름 (사용자 입력)
	int m_nThickness;                  // 정원 테두리 두께 (사용자 입력)
	bool m_bDragging;                  // 드래그 중인지 여부
	int m_nDragPointIndex;             // 드래그 중인 클릭 지점의 인덱스 (0, 1, 2)
	ULONG_PTR m_gdiplusToken;

	CWinThread* m_pRandomMoveThread;
	static UINT RandomMoveThreadProc(LPVOID pParam); 
	CEdit m_editRadius;
	CEdit m_editThickness;
	CStatic m_staticPoint1;
	CStatic m_staticPoint2;
	CStatic m_staticPoint3;
	void DrawCirclePixelByPixel(CDC* pDC, CPoint center, int radius, COLORREF color, int penWidth);
	bool CalculateCircleFromThreePoints(const CPoint& p1, const CPoint& p2, const CPoint& p3, CPoint& center, int& radius);
};
