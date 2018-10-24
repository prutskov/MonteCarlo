
// MonteCarloDlg.cpp: файл реализации
//

#include "stdafx.h"
#include "MonteCarlo.h"
#include "MonteCarloDlg.h"
#include "afxdialogex.h"
#include <ctime>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Диалоговое окно CMonteCarloDlg

CRITICAL_SECTION cs;

CMonteCarloDlg::CMonteCarloDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MONTECARLO_DIALOG, pParent)
	, _N(50)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMonteCarloDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MODEL, _drawerModel);
	DDX_Text(pDX, IDC_EDIT1, _N);
}

BEGIN_MESSAGE_MAP(CMonteCarloDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_RUN, &CMonteCarloDlg::OnBnClickedRun)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// Обработчики сообщений CMonteCarloDlg

BOOL CMonteCarloDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Задает значок для этого диалогового окна.  Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	// TODO: добавьте дополнительную инициализацию
	InitializeCriticalSection(&cs);
	srand(time(NULL));
	_drawerModel._points = &_points;

	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок.  Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CMonteCarloDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CMonteCarloDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMonteCarloDlg::InitializeModel()
{
	int width = _N + 2, height = _N + 2;
	_points.clear();
	for (int h = 0; h < height; h++)
	{
		std::vector<bool> row;
		for (int w = 0; w < width; w++)
		{
			row.push_back(rand() % 2 == true);
		}
		_points.push_back(row);
	}
	_T = 0.5* Tc();
	_w[0] = std::exp(-8 * std::abs(J) / k / _T);
	_w[1] = std::exp(-4 * std::abs(J) / k / _T);
}

void CMonteCarloDlg::OnBnClickedRun()
{
	/*UpdateData(TRUE);
	InitializeModel();
	SetTimer(1, 1, NULL);
	_drawerModel.RedrawWindow();*/
	UpdateData(TRUE);
	InitializeModel();
	SetTimer(1, 50, NULL);
	hThreadCalculate = CreateThread(
		NULL,		// дескриптор защиты
		0,			// начальный размер стека ( Если это значение нулевое, новый поток использует по умолчанию размер стека исполняемой программы)
		(LPTHREAD_START_ROUTINE)ThreadCalculate,	 // функция потока
		this,		// параметр потока 
		0,			//oпции создания(здесь можно отложить запуск выполнения потока. Для запуска потока сразу же, передаём 0.)
		&pdwThreadCalculate);// идентификатор потока (указатель на переменную, куда будет сохранён идентификатор потока)
}

//int CMonteCarloDlg::dE(int i, int j)
//{
//	int s = spin_at(i, j - 1) + spin_at(i, j + 1) +
//		spin_at(i - 1, j) + spin_at(i + 1, j);
//	s *= J < 0 ? -1 : 1;
//	return -spin_at(i, j) * s / 2 + 2;
//}
int CMonteCarloDlg::E()
{
	int s = 0;
#pragma omp parallel for reduction(+:s)
	for (int i = 1; i < _N + 1; ++i)

		for (int j = 1; j < _N + 1; ++j)
		{
			s += spin_at(i, j) * (spin_at(i - 1, j) + spin_at(i, j - 1));
		}
	return -s*J;
}
int CMonteCarloDlg::spin_at(size_t i, size_t j)
{
	return _points[i][j] ? 1 : -1;
}

void CMonteCarloDlg::ensure_periodic()
{
#pragma omp parallel for
	for (int i = 0; i < _N + 2; ++i)
	{
		_points[i][0] = _points[i][_N];
		_points[i][_N + 1] = _points[i][1];
		_points[0][i] = _points[_N][i];
		_points[_N + 1][i] = _points[1][i];
	}
}

void CMonteCarloDlg::ensure_periodic(size_t i, size_t j)
{
	_points[i][0] = _points[i][_N];
	_points[i][_N + 1] = _points[i][1];
	_points[0][i] = _points[_N][i];
	_points[_N + 1][i] = _points[1][i];
}


//void CMonteCarloDlg::next_linear(size_t n)
//{
//	for (size_t l = 0; l < n * n; ++l)
//	{
//		size_t i = (rand() % (n - 2)) + 2;
//		size_t j = (rand() % (n - 2)) + 2;
//		size_t c = dE(i, j);
//
//		int horizontalRnd, verticRnd;
//		horizontalRnd = rand() % 2 == 1 ? 1 : -1;
//		verticRnd = rand() % 2 == 1 ? 1 : -1;
//		if (_points[i][j] == _points[i + verticRnd][j + horizontalRnd])
//		{
//			continue;
//		}
//		
//
//		if (c >= 2)
//		{
//			bool buf = _points[i][j];
//			_points[i][j] = _points[i + verticRnd][j + horizontalRnd];
//			_points[i + verticRnd][j + horizontalRnd] = buf;
//		}
//		else
//		{
//			if (rand() < _w[c] * RAND_MAX)
//			{
//				bool buf = _points[i][j];
//				_points[i][j] = _points[i + verticRnd][j + horizontalRnd];
//				_points[i + verticRnd][j + horizontalRnd] = buf;
//			}
//		}
//		ensure_periodic(i, j);
//	}
//
//
//	for (int h = 1; h < _N + 1; h++)
//	{
//		for (int w = 1; w < _N + 1; w++)
//		{
//			int horizontalRnd, verticRnd;
//			horizontalRnd = rand() % 2 == 1 ? 1 : -1;
//			verticRnd = rand() % 2 == 1 ? 1 : -1;
//			while (true)
//			{
//				if (((w + horizontalRnd) < 1) ||
//					((w + horizontalRnd) > _N))
//				{
//					horizontalRnd = rand() % 2 == 1 ? 1 : -1;
//				}
//				else break;
//			}
//			while (true)
//			{
//				if (((h + verticRnd) < 1) ||
//					((h + verticRnd) > _N))
//				{
//					verticRnd = rand() % 2 == 1 ? 1 : -1;
//				}
//				else break;
//			}
//
//			size_t c = dE(h + verticRnd, w + horizontalRnd);
//			if (_points[h][w] == _points[h + verticRnd][w + horizontalRnd])
//			{
//				continue;
//			}
//			if (c >= 2)
//			{
//				bool buf = _points[h][w];
//				_points[h][w] = _points[h + verticRnd][w + horizontalRnd];
//				_points[h + verticRnd][w + horizontalRnd] = buf;
//			}
//			else
//			{
//				if (rand() < _w[c] * RAND_MAX)
//				{
//					bool buf = _points[h][w];
//					_points[h][w] = _points[h + verticRnd][w + horizontalRnd];
//					_points[h + verticRnd][w + horizontalRnd] = buf;
//				}
//			}
//			ensure_periodic(h, w);
//		}
//	}
//}

void CMonteCarloDlg::next_linear(size_t n)
{
//#pragma omp for
	for (int l = 0; l < n * n; ++l)
	{
		size_t i = (rand() % (n - 2)) + 2;
		size_t j = (rand() % (n - 2)) + 2;

		
		int horizontalRnd, verticRnd;
		horizontalRnd = rand() % 2 == 1 ? 1 : -1;
		verticRnd = rand() % 2 == 1 ? 1 : -1;
		if (_points[i][j] == _points[i + verticRnd][j + horizontalRnd])
		{
			continue;
		}
		int energyBegin = E();

		const bool ij = _points[i][j];
		const bool i2j2 = _points[i + verticRnd][j + horizontalRnd];

		_points[i][j] = _points[i + verticRnd][j + horizontalRnd];
		_points[i + verticRnd][j + horizontalRnd] = ij;

		int energyEnd = E();


		if ((energyEnd-energyBegin)>0)
		{
			if ((double)rand()/RAND_MAX > exp(-(energyEnd - energyBegin)/k/_T))
			{
				_points[i][j] = ij;
				_points[i + verticRnd][j + horizontalRnd] = i2j2;
			}
		}
		ensure_periodic(i, j);
	}	
}

void CMonteCarloDlg::next()
{
	int s = 0;

	const size_t n = _N;

	next_linear(n);

}

void CMonteCarloDlg::OnTimer(UINT_PTR nIDEvent)
{
	//EnterCriticalSection(&cs);
	_drawerModel.RedrawWindow();
	//LeaveCriticalSection(&cs);

	CDialogEx::OnTimer(nIDEvent);
}

DWORD WINAPI ThreadCalculate(PVOID param)
{
	CMonteCarloDlg *dlg = (CMonteCarloDlg*)param;
	while (true)
	{
		EnterCriticalSection(&cs);
		dlg->next();
		LeaveCriticalSection(&cs);
	}
	return 0;
}
