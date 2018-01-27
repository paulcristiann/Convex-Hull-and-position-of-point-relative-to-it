#include <GL/freeglut.h>
#include <math.h>
#include <iostream>
#include <stack>
#include <stdlib.h>
#include <vector>

using namespace std;

bool valid;

struct Point
{
	float x, y;
};

void circle(vector<Point> v, vector<Point> t, Point P)
{
	float x = 0.0; float y = 0.0;
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //negru si opac
	glClear(GL_COLOR_BUFFER_BIT);
	int number = 40;
	float radius = 0.1;
	float twopi = 2.0 * 3.1452423;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//desenam cercurile
	for (int i = 0; i < t.size(); i++)
	{
		bool eInAcoperire = 0;
		for (int j = 0; j < v.size(); j++)
			if (v[j].x == t[i].x && v[j].y == t[i].y)
				eInAcoperire = 1;

		glBegin(GL_TRIANGLE_FAN);
		//glBegin(GL_POLYGON);
		if (eInAcoperire)
			glColor3f(0.0f, 1.0f, 0.0f);
		else
			glColor3f(1.0f, 0.0f, 0.0f);
		x = float(t[i].x) / 10; y = float(t[i].y) / 10;
		glVertex2f(x, y);
		for (int i = 0; i <= 40; i++)
			glVertex2f(x + radius * cosf(i*twopi / number), y + radius * sinf(i*twopi / number));
		glEnd();
	}

	//desenam frontiera
	for (int i = 0; i < v.size(); i++)
	{
		glLineWidth(2.5);
		glColor3f(0.0, 1.0, 0.0);
		glBegin(GL_LINES);
		glVertex2f(float(v[i].x) / 10, float(v[i].y) / 10);
		if (i == v.size() - 1)
			glVertex2f(float(v[0].x) / 10, float(v[0].y) / 10);
		else
			glVertex2f(float(v[i + 1].x) / 10, float(v[i + 1].y) / 10);
		glEnd();
	}

	//desenam punctul
	radius = 0.03;
	glBegin(GL_TRIANGLE_FAN);
	//glBegin(GL_POLYGON);
	glColor3f(0.0f, 0.0f, 1.0f);
	x = float(P.x) / 10; y = float(P.y) / 10;
	glVertex2f(x, y);
	for (int i = 0; i <= 40; i++)
		glVertex2f(x + radius * cosf(i*twopi / number), y + radius * sinf(i*twopi / number));
	glEnd();

	glFlush();
}

Point p0;
vector<Point> acoperire;
bool ok;

// Functie care returneaza penultimul punct selectat din stiva
Point nextToTop(stack<Point> &S)
{
	Point p = S.top();
	S.pop();
	Point res = S.top();
	S.push(p);
	return res;
}

// Inversarea a doua puncte
void swap(Point &p1, Point &p2)
{
	Point temp = p1;
	p1 = p2;
	p2 = temp;
}

// Functie care returneaza patratul distantei
// dintre p1 si p2
int distSq(Point p1, Point p2)
{
	return (p1.x - p2.x)*(p1.x - p2.x) +
		(p1.y - p2.y)*(p1.y - p2.y);
}

int orientation(Point p, Point q, Point r)
{
	int val = (q.y - p.y) * (r.x - q.x) -
		(q.x - p.x) * (r.y - q.y);

	if (val == 0) return 0;  // coliniare
	return (val > 0) ? 1 : 2; // sens invers trigonometric sau trigonometric
}

int compare(const void *vp1, const void *vp2)
{
	Point *p1 = (Point *)vp1;
	Point *p2 = (Point *)vp2;

	int o = orientation(p0, *p1, *p2);
	if (o == 0)
		return (distSq(p0, *p2) >= distSq(p0, *p1)) ? -1 : 1;

	return (o == 2) ? -1 : 1;
}

void acopConvexa(vector<Point> points, int n)
{
	// gasim cel mai de jos punct
	int ymin = points[0].y, min = 0;
	for (int i = 1; i < n; i++)
	{
		int y = points[i].y;

		//in caz de egalitate il luam pe cel mai din stanga
		if ((y < ymin) || (ymin == y &&
			points[i].x < points[min].x))
			ymin = points[i].y, min = i;
	}

	// Il asezam pe prima pozitie in vector
	swap(points[0], points[min]);

	// Sortam cele n-1 puncte crescator dupa unghiul polar
	p0 = points[0];
	qsort(&points[1], n - 1, sizeof(Point), compare);

	// Daca 2 sau mai multe puncte formeaza acelasi unghi cu primul, il pastram doar pe cel mai indepartat
	int m = 1; // Contorul vectorului dupa eliminari
	for (int i = 1; i<n; i++)
	{
		// Il stergem pe i atata timp cat are acelasi unghi ca i+1
		while (i < n - 1 && orientation(p0, points[i], points[i + 1]) == 0)
			i++;


		points[m] = points[i];
		m++;
	}

	// Daca m < 3, punctele sunt coliniare
	// acoperirea sunt capetele segmentului format de puncte
	if (m < 3)
	{
		ok = 1;
		return;
	}

	// Bagam primele 3 puncte in stiva
	stack<Point> S;
	S.push(points[0]);
	S.push(points[1]);
	S.push(points[2]);

	for (int i = 3; i < m; i++)
	{
		// Eliminam varful cat timp primele 2 elemente de pe stiva si cele ramase nu fac viraj la stanga
		while (orientation(nextToTop(S), S.top(), points[i]) != 2)
			S.pop();
		S.push(points[i]);
	}

	while (!S.empty())
	{
		Point p = S.top();
		acoperire.push_back(p);
		S.pop();
	}
}

bool determinant(float x1, float y1, float x2, float y2, float xp, float yp)
{
	float nr = x1 * y2 + yp * x2 + xp * y1 - (xp*y2 + x1 * yp + y1 * x2);
	if (nr >= 0)
		return 1;
	else
		return 0;
}

float distantaPanaLaDreapta(float x, float y, float ax, float ay, float bx, float by)
{
	float a = ay - by;
	float b = bx - ax;
	float c = ax*by - bx * ay;
	float dist = abs(a * x + b * y + c) / sqrt(a*a + b * b);
	std::cout << a << " " << b << " " << c << endl;
	return dist;
}

bool proiectiaEpeSegment(float x, float y, float ax, float ay, float bx, float by)
{
	float a = ay - by;
	float b = bx - ax;
	float c = ax * by - bx * ay;
	float pantaSegment = -a / b;
	float pantaDreapta = -1 / pantaSegment;
	float aDreapta = pantaDreapta;
	float bDreapta = -1;
	float cDreapta = pantaDreapta * x - y;
	//std::cout << aDreapta << " " << bDreapta << " " << cDreapta << endl;
	float xIntersectie = -(cDreapta*b - c * bDreapta) / (a * bDreapta - aDreapta * b);
	float yIntersectie = (-c - a * xIntersectie) / b;

	std::cout << xIntersectie << " " << yIntersectie << endl;
	
	if (xIntersectie >= ax && xIntersectie <= bx && yIntersectie >= ay && yIntersectie <= by)
		return 1;
	else
		return 0;
}

float distantaIntreDouaPuncte(float x1, float y1, float x2, float y2)
{
	return sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
}

int main(int argc, char**argv)
{
	//std::cout<<proiectiaEpeSegment(3, 3, 1, 1, 5, 5);
	////distantaPanaLaDreapta(1, 2.5, 1, 1, 3, 3);
	////std::cout << distantaPanaLaDreapta(7, 7, 2, 3, -1, 1);
	vector<Point> points;
	int n;
	std::cout << "Cate cercuri se vor citi? n=";
	cin >> n;
	for (int i = 0; i < n; i++)
	{
		std::cout << "Coordonatele cercului nr " << i + 1 << endl;
		std::cout << "x="; float x; cin >> x;
		std::cout << "y="; float y; cin >> y;
		Point Z; Z.x = x; Z.y = y;
		points.push_back(Z);
	}

	Point P;
	std::cout << "Punctul pentru care se decide apartenenta" << endl;
	std::cout << "x="; cin >> P.x;
	std::cout << "y="; cin >> P.y;
	//vector<Point>I;
	//vector<Point>J;
	acopConvexa(points, n);
	if (ok == 1)
	{
		std::cout << "Coliniare" << endl;

		//std::cout<<"I={("<<a.x<<","<<a.y<<"),("<<d.x<<","<<d.y<<")}"<<endl;
		//std::cout<<"J={("<<b.x<<","<<b.y<<"),("<<c.x<<","<<c.y<<")}"<<endl;
		return 0;
	} 
	if (acoperire.size() == 3)
	{
		std::cout << "Triunghi" << endl;
		//        for(int i=0; i<4; i++)
		//        {
		//            bool gasit = 0;
		//            for(int j=0; j<3; j++)
		//                if(points[i].x == acoperire[j].x && points[i].y == acoperire[j].y)
		//                    gasit = 1;
		//            if(gasit == 0)
		//            {
		//                I.push_back(points[i]);
		//                break;
		//            }
		//        }
		//        for(int i=0; i<3; i++)
		//            J.push_back(acoperire[i]);
		bool semnPrecedent = 0;
		bool eInAcoperire = 1;
		for (int i = 0; i < acoperire.size(); i++)
		{
			bool semn;
			if (i == 0)
			{
				semn = determinant(acoperire[i].x, acoperire[i].y, acoperire[i + 1].x, acoperire[i + 1].y, P.x, P.y);
				semnPrecedent = determinant(acoperire[i].x, acoperire[i].y, acoperire[i + 1].x, acoperire[i + 1].y, P.x, P.y);
			}
			else
				if (i == acoperire.size() - 1)
				{
					semn = determinant(acoperire[i].x, acoperire[i].y, acoperire[0].x, acoperire[0].y, P.x, P.y);
				}
				else
				{
					semn = determinant(acoperire[i].x, acoperire[i].y, acoperire[i + 1].x, acoperire[i + 1].y, P.x, P.y);
				}
			
			if (semn != semnPrecedent)
			{
				eInAcoperire = 0;
			}
		}

		if (eInAcoperire == 0)
		{
			for (int andreea = 0; andreea < acoperire.size(); andreea++)
				if (distantaIntreDouaPuncte(acoperire[andreea].x, acoperire[andreea].y, P.x, P.y) <= 1)
					eInAcoperire = 1;
		}

		if (eInAcoperire == 0)
		{
			for (int andreea = 0; andreea < acoperire.size(); andreea++)
			{
				if (andreea == acoperire.size() - 1)
				{
					float d = distantaPanaLaDreapta(P.x, P.y, acoperire[andreea].x, acoperire[andreea].y, acoperire[0].x, acoperire[0].y);
					if (proiectiaEpeSegment(P.x, P.y, acoperire[andreea].x, acoperire[andreea].y, acoperire[0].x, acoperire[0].y) && d <= 1)
						eInAcoperire = 1;
				}
				else
				{
					float d = distantaPanaLaDreapta(P.x, P.y, acoperire[andreea].x, acoperire[andreea].y, acoperire[andreea+1].x, acoperire[andreea+1].y);
					if (proiectiaEpeSegment(P.x, P.y, acoperire[andreea].x, acoperire[andreea].y, acoperire[andreea + 1].x, acoperire[andreea + 1].y) && d <= 1)
						eInAcoperire = 1;
				}
			}
		}

		if (eInAcoperire)
			std::cout << "Punctul se afla in acoperire!" << endl;
		else
			std::cout << "E in plm" << endl;

		glutInit(&argc, argv);
		glutInitWindowSize(900, 800);
		glutInitWindowPosition(50, 50);
		glutCreateWindow("Reprezentare grafica");

		circle(acoperire, points, P);

		glutMainLoop();
		return 0;
	}
	else
	{
		std::cout << "Poligon oarecare" << endl;
		bool semnPrecedent = 0;
		bool eInAcoperire = 1;
		for (int i = 0; i < acoperire.size(); i++)
		{
			bool semn;
			if (i == 0)
			{
				semn = determinant(acoperire[i].x, acoperire[i].y, acoperire[i + 1].x, acoperire[i + 1].y, P.x, P.y);
				semnPrecedent = determinant(acoperire[i].x, acoperire[i].y, acoperire[i + 1].x, acoperire[i + 1].y, P.x, P.y);
			}
			else
				if (i == acoperire.size() - 1)
				{
					semn = determinant(acoperire[i].x, acoperire[i].y, acoperire[0].x, acoperire[0].y, P.x, P.y);
				}
				else
				{
					semn = determinant(acoperire[i].x, acoperire[i].y, acoperire[i + 1].x, acoperire[i + 1].y, P.x, P.y);
				}

			if (semn != semnPrecedent)
			{
				eInAcoperire = 0;
			}
		}

		if (eInAcoperire == 0)
		{
			for (int andreea = 0; andreea < acoperire.size(); andreea++)
				if (distantaIntreDouaPuncte(acoperire[andreea].x, acoperire[andreea].y, P.x, P.y) <= 1)
					eInAcoperire = 1;
		}

		if (eInAcoperire == 0)
		{
			for (int andreea = 0; andreea < acoperire.size(); andreea++)
			{
				if (andreea == acoperire.size() - 1)
				{
					float d = distantaPanaLaDreapta(P.x, P.y, acoperire[andreea].x, acoperire[andreea].y, acoperire[0].x, acoperire[0].y);
					if (proiectiaEpeSegment(P.x, P.y, acoperire[andreea].x, acoperire[andreea].y, acoperire[0].x, acoperire[0].y) && d <= 1)
						eInAcoperire = 1;
				}
				else
				{
					float d = distantaPanaLaDreapta(P.x, P.y, acoperire[andreea].x, acoperire[andreea].y, acoperire[andreea + 1].x, acoperire[andreea + 1].y);
					if (proiectiaEpeSegment(P.x, P.y, acoperire[andreea].x, acoperire[andreea].y, acoperire[andreea + 1].x, acoperire[andreea + 1].y) && d <= 1)
						eInAcoperire = 1;
				}
			}
		}

		if (eInAcoperire)
			std::cout << "Punctul se afla in acoperire!" << endl;
		else
			std::cout << "Nu e in acoperire" << endl;

		glutInit(&argc, argv);
		glutInitWindowSize(900, 800);
		glutInitWindowPosition(50, 50);
		glutCreateWindow("Reprezentare grafica");

		circle(acoperire, points, P);

		glutMainLoop();
		return 0;
	}
	
	/*for(int i = 0; i<acoperire.size(); i++)
	{
		std::cout << acoperire[i].x << " " << acoperire[i].y << endl;
	}*/

	return 0;
}