
// AssignmentDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "Assignment.h"
#include "AssignmentDlg.h"
#include "afxdialogex.h"
#include <random>
#include <chrono>
#include <limits>
#include <cmath>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAssignmentDlg 대화 상자



CAssignmentDlg::CAssignmentDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ASSIGNMENT_DIALOG, pParent), m_clickCount(0)
	, m_nRadius(20) // 초기 반지름 기본값
	, m_nThickness(2) // 초기 테두리 두께 기본값
	, m_bDragging(false)
	, m_nDragPointIndex(-1)
	, m_gdiplusToken(0) 
	, m_pRandomMoveThread(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAssignmentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Text(pDX, IDC_EDIT_RADIUS, m_editRadius);
	DDX_Control(pDX, IDC_EDIT_RADIUS, m_editRadius);
	DDX_Control(pDX, IDC_EDIT_THICKNESS, m_editThickness);
	DDX_Control(pDX, IDC_STATIC_POINT1, m_staticPoint1);
	DDX_Control(pDX, IDC_STATIC_POINT2, m_staticPoint2);
	DDX_Control(pDX, IDC_STATIC_POINT3, m_staticPoint3);
}

BEGIN_MESSAGE_MAP(CAssignmentDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_RESET, &CAssignmentDlg::OnBnClickedBtnReset)
	ON_BN_CLICKED(IDC_BTN_RANDOM_MOVE, &CAssignmentDlg::OnBnClickedBtnRandomMove)
	ON_MESSAGE(WM_MY_UPDATE_CIRCLE, &CAssignmentDlg::OnMyUpdateCircle)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CAssignmentDlg 메시지 처리기

BOOL CAssignmentDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.
	
	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
	m_editRadius.SetWindowText(_T("20"));
	m_editThickness.SetWindowText(_T("2"));
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CAssignmentDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CAssignmentDlg::OnPaint()
{
	CPaintDC dc(this); 
	CRect clientRect;
	GetClientRect(&clientRect);
	CDC memDC;
	CBitmap bitmap;
	memDC.CreateCompatibleDC(&dc);
	bitmap.CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height());
	memDC.SelectObject(&bitmap);
	memDC.FillSolidRect(clientRect, RGB(255, 255, 255));
	CString strRadius;
	m_editRadius.GetWindowText(strRadius);
	m_nRadius = _ttoi(strRadius);
	if (m_nRadius <= 0) m_nRadius = 1; 

	for (int i = 0; i < m_points.size(); ++i)
	{
		if (i < 3)
		{
			//DrawCirclePixelByPixel(&memDC, m_points[i], m_nRadius, RGB(0, 0, 0),2); 
			CPoint center = m_points[i];
			int radius = m_nRadius;

			for (int y = -radius; y <= radius; ++y)
			{
				for (int x = -radius; x <= radius; ++x)
				{
					if (x * x + y * y <= radius * radius)
					{
						memDC.SetPixel(center.x + x, center.y + y, RGB(0, 0, 0)); // 검은색 채움
					}
				}
			}
		}
	}
	if (m_points.size() >= 3)
	{
		CPoint circleCenter;
		int circleRadius;

		CString strThickness;
		m_editThickness.GetWindowText(strThickness);
		m_nThickness = _ttoi(strThickness);
		if (m_nThickness <= 0) m_nThickness = 1; 

		if (CalculateCircleFromThreePoints(m_points[0], m_points[1], m_points[2], circleCenter, circleRadius))
		{
			DrawCirclePixelByPixel(&memDC, circleCenter, circleRadius, RGB(0, 0, 255), m_nThickness);
		}
	}
	dc.BitBlt(0, 0, clientRect.Width(), clientRect.Height(), &memDC, 0, 0, SRCCOPY);

	memDC.DeleteDC();
	bitmap.DeleteObject();
}

HCURSOR CAssignmentDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CAssignmentDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialogEx::OnLButtonDown(nFlags, point);
	if (m_points.size() >= 3)
	{
		CString strRadius;
		m_editRadius.GetWindowText(strRadius);
		int clickRadius = _ttoi(strRadius);
		if (clickRadius <= 0) clickRadius = 1;

		for (int i = 0; i < 3; ++i)
		{

			double distance = sqrt(pow(point.x - m_points[i].x, 2) + pow(point.y - m_points[i].y, 2));
			if (distance <= clickRadius) 
			{
				m_bDragging = true;
				m_nDragPointIndex = i;
				SetCapture(); 
				return;
			}
		}
	}

	if (m_clickCount < 3)
	{
		m_points.push_back(point);
		m_clickCount++;

		CString strPoint;
		strPoint.Format(_T("X: %d, Y: %d"), point.x, point.y);
		switch (m_clickCount)
		{
		case 1: m_staticPoint1.SetWindowText(strPoint); break;
		case 2: m_staticPoint2.SetWindowText(strPoint); break;
		case 3: m_staticPoint3.SetWindowText(strPoint); break;
		}
		Invalidate(); 
	}
}


void CAssignmentDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CDialogEx::OnMouseMove(nFlags, point);
	if (m_bDragging && (nFlags & MK_LBUTTON))
	{
		if (m_nDragPointIndex != -1)
		{
			m_points[m_nDragPointIndex] = point;
			Invalidate(); 
			CString strPoint;
			strPoint.Format(_T("X: %d, Y: %d"), point.x, point.y);
			switch (m_nDragPointIndex)
			{
			case 0: m_staticPoint1.SetWindowText(strPoint); break;
			case 1: m_staticPoint2.SetWindowText(strPoint); break;
			case 2: m_staticPoint3.SetWindowText(strPoint); break;
			}
		}
	}
}


void CAssignmentDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDialogEx::OnLButtonUp(nFlags, point);

	if (m_bDragging)
	{
		m_bDragging = false;
		m_nDragPointIndex = -1;
		ReleaseCapture(); 
	}
}

void CAssignmentDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	if (m_gdiplusToken != 0) {
		Gdiplus::GdiplusShutdown(m_gdiplusToken);
		m_gdiplusToken = 0;
	}
}


void CAssignmentDlg::OnBnClickedBtnReset()
{
	m_points.clear();      
	m_clickCount = 0;      
	m_bDragging = false;
	m_nDragPointIndex = -1;

	m_staticPoint1.SetWindowText(_T(""));
	m_staticPoint2.SetWindowText(_T(""));
	m_staticPoint3.SetWindowText(_T(""));

	if (m_pRandomMoveThread)
	{
		if (::WaitForSingleObject(m_pRandomMoveThread->m_hThread, 0) == WAIT_TIMEOUT)
		{
			::WaitForSingleObject(m_pRandomMoveThread->m_hThread, INFINITE);
		}
		m_pRandomMoveThread = nullptr;
	}

	Invalidate(); 
}



void CAssignmentDlg::OnBnClickedBtnRandomMove()
{
    if (m_points.size() < 3)
    {
        AfxMessageBox(_T("세 개의 지점이 있어야 합니다."), MB_ICONINFORMATION | MB_OK);
        return;
    }

    if (m_pRandomMoveThread != nullptr && ::WaitForSingleObject(m_pRandomMoveThread->m_hThread, 0) == WAIT_TIMEOUT)
    {
        AfxMessageBox(_T("랜덤이동 진행중"), MB_ICONINFORMATION | MB_OK);
        return;
    }
    m_pRandomMoveThread = AfxBeginThread(RandomMoveThreadProc, this);
    if (!m_pRandomMoveThread)
    {
        AfxMessageBox(_T("쓰래드 생성 실패"), MB_ICONERROR | MB_OK);
    }
}

UINT CAssignmentDlg::RandomMoveThreadProc(LPVOID pParam)
{
	CAssignmentDlg* pThis = static_cast<CAssignmentDlg*>(pParam);
	if (!pThis) return 1;

	std::mt19937 rng(std::chrono::system_clock::now().time_since_epoch().count());

	CRect clientRect;
	pThis->GetClientRect(&clientRect); 

	std::uniform_int_distribution<int> distX(0, clientRect.Width());
	std::uniform_int_distribution<int> distY(0, clientRect.Height());

	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			pThis->m_points[j].x = distX(rng);
			pThis->m_points[j].y = distY(rng);
		}

		pThis->PostMessage(WM_MY_UPDATE_CIRCLE, 0, 0);

		for (int j = 0; j < 3; ++j) {
			CString strPoint;
			strPoint.Format(_T("X: %d, Y: %d"), pThis->m_points[j].x, pThis->m_points[j].y);
			switch (j) {
			case 0: pThis->m_staticPoint1.SetWindowText(strPoint); break;
			case 1: pThis->m_staticPoint2.SetWindowText(strPoint); break;
			case 2: pThis->m_staticPoint3.SetWindowText(strPoint); break;
			}
		}


		Sleep(500);
	}

	pThis->m_pRandomMoveThread = nullptr; 
	return 0;
}
void CAssignmentDlg::DrawCirclePixelByPixel(CDC* pDC, CPoint center, int radius, COLORREF color, int penWidth)
{
	if (penWidth < 1) penWidth = 1;
	int x = 0;
	int y = radius;
	int d = 3 - 2 * radius;

	auto plotCirclePoints = [&](int cx, int cy, int x_offset, int y_offset, int width) {
		for (int i = 0; i < width; ++i)
		{
			for (int j = 0; j < width; ++j)
			{
				pDC->SetPixel(cx + x_offset + i, cy + y_offset + j, color);
				pDC->SetPixel(cx - x_offset - i, cy + y_offset + j, color);
				pDC->SetPixel(cx + x_offset + i, cy - y_offset - j, color);
				pDC->SetPixel(cx - x_offset - i, cy - y_offset - j, color);

				pDC->SetPixel(cx + y_offset + i, cy + x_offset + j, color);
				pDC->SetPixel(cx - y_offset - i, cy + x_offset + j, color);
				pDC->SetPixel(cx + y_offset + i, cy - x_offset - j, color);
				pDC->SetPixel(cx - y_offset - i, cy - x_offset - j, color);
			}
		}
	};

	while (y >= x)
	{
		plotCirclePoints(center.x, center.y, x, y, penWidth);
		x++;
		if (d > 0)
		{
			y--;
			d = d + 4 * (x - y) + 10;
		}
		else
		{
			d = d + 4 * x + 6;
		}
	}
}
bool CAssignmentDlg::CalculateCircleFromThreePoints(const CPoint& p1, const CPoint& p2, const CPoint& p3, CPoint& center, int& radius)
{
	double midX12 = (double)(p1.x + p2.x) / 2.0;
	double midY12 = (double)(p1.y + p2.y) / 2.0;
	double slope12;

	if (p2.x - p1.x == 0)
	{
		slope12 = std::numeric_limits<double>::infinity();
	}
	else
	{
		slope12 = (double)(p2.y - p1.y) / (double)(p2.x - p1.x);
	}

	double perpSlope12 = (slope12 == 0) ? std::numeric_limits<double>::infinity() : -1.0 / slope12;
	double midX23 = (double)(p2.x + p3.x) / 2.0;
	double midY23 = (double)(p2.y + p3.y) / 2.0;
	double slope23;

	if (p3.x - p2.x == 0) 
	{
		slope23 = std::numeric_limits<double>::infinity();
	}
	else
	{
		slope23 = (double)(p3.y - p2.y) / (double)(p3.x - p2.x);
	}

	double perpSlope23 = (slope23 == 0) ? std::numeric_limits<double>::infinity() : -1.0 / slope23;

	double cx, cy;
	if (perpSlope12 == perpSlope23) {
		return false; 
	}

	if (std::isfinite(perpSlope12) && std::isfinite(perpSlope23))
	{
		cx = (midY23 - midY12 + perpSlope12 * midX12 - perpSlope23 * midX23) / (perpSlope12 - perpSlope23);
		cy = perpSlope12 * (cx - midX12) + midY12;
	}
	else if (std::isinf(perpSlope12) && std::isfinite(perpSlope23))
	{
		cx = midX12;
		cy = perpSlope23 * (cx - midX23) + midY23;
	}
	else if (std::isfinite(perpSlope12) && std::isinf(perpSlope23))
	{
		cx = midX23;
		cy = perpSlope12 * (cx - midX12) + midY12;
	}

	else
	{
		return false; 
	}

	center = CPoint(static_cast<int>(round(cx)), static_cast<int>(round(cy)));

	radius = static_cast<int>(round(sqrt(pow(p1.x - center.x, 2) + pow(p1.y - center.y, 2))));
	return true;
}

LRESULT CAssignmentDlg::OnMyUpdateCircle(WPARAM wParam, LPARAM lParam)
{
	Invalidate();
	return 0;
}