
// MonteCarloDlg.h: файл заголовка
//

#pragma once
#include "Drawer2D.h"

// Диалоговое окно CMonteCarloDlg
class CMonteCarloDlg : public CDialogEx
{
// Создание
public:
	CMonteCarloDlg(CWnd* pParent = nullptr);	// стандартный конструктор

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MONTECARLO_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV

	const double k = 8.617e-5;
	double J = 1;
	double Tc() const { return 2.269 * std::abs(J) / k; }
	double _w[2];
	double _T;


// Реализация
protected:
	HICON m_hIcon;

	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	Drawer2D _drawerModel;
	void InitializeModel();
	int dE(int i, int j);
	int spin_at(size_t i, size_t j);
	void ensure_periodic();
	void ensure_periodic(size_t i, size_t j);
	void next_linear(size_t n);
	void next();
	std::vector<std::vector<bool>> _points;

	int _N;
	afx_msg void OnBnClickedRun();
};
