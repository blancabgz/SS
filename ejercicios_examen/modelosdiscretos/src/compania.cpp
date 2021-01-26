#include "compania.h"
#include <chrono>
#include <random>
#include <iostream>

using namespace std;

bool compare(const suc &s1, const suc &s2){
	return s1.tiempo < s2.tiempo;
}

Compania::Compania(){}

double Compania::gendem(double media){
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
 	std::default_random_engine gen (seed);
 	std::exponential_distribution<double> dist (media);

	return dist(gen);
}

double Compania::genpedido(double min, double max){
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine gen (seed);
	uniform_real_distribution<double> dist(min, max);
	return dist(gen);
}

void Compania::tabla_de_propabilidad(){
	probabilidad.push_back(1.0/6.0);
	for (int i = 1; i < 3; i++){
		probabilidad.push_back(probabilidad[i-1] + 1.0/3.0);
	}
	probabilidad.push_back(probabilidad[(int)probabilidad.size() - 1] + 1.0/6.0);
}

double Compania::genera_tamano(){
	double u = genpedido(0.0, 1.0);

	bool encontrado = false;
	int prob = 0;
	for(int i = 0; i < (int)probabilidad.size() && !encontrado; i++){
		//cout << "P: " << u << ", " << probabilidad[i] << endl;
		if ( u <= probabilidad[i] ){
			prob = i;
			encontrado = true;
		}
	}
	//cout << "Probabilidad: " << u << ", " << prob << endl;
	return prob + 1;
}

void Compania::insertar_lsuc(suc n){
	lsuc.push_back(n);
	lsuc.sort(compare);
}

void Compania::inicializacion(){
	reloj = 0.0;
	tparada = 120.0;
	tam = 0.0;
	nivel = 0.0;
	pedido = 0.0;
	tultsuc = 0.0;

	acummas = 0.0;
	acummenos = 0.0;
	acumpedido = 0.0;

	tabla_de_propabilidad();

	while(!lsuc.empty()){
		lsuc.pop_front();
	}

	reg_cola_null.tiempo = 0.0;
	reg_cola_null.tipo = 0;
	nodo.reg_cola = reg_cola_null;
	nodo.suceso = SUCESO_FIN_SIMULACION;
	nodo.tiempo = reloj+tparada;
	insertar_lsuc(nodo);

	nodo.suceso = SUCESO_DEMANDA;
	nodo.tiempo = reloj+gendem(0.1);
	insertar_lsuc(nodo);

	nodo.suceso = SUCESO_EVAL;
	nodo.tiempo = reloj+1.0;
	insertar_lsuc(nodo);

	parar = false;
}

void Compania::temporizacion(){
	nodo = lsuc.front();
	lsuc.pop_front();
	reloj = nodo.tiempo;
}

void Compania::suceso(){
	if(nodo.suceso == SUCESO_DEMANDA){
		demanda();
	} else if(nodo.suceso == SUCESO_EVAL){
		evaluacion();
	} else if(nodo.suceso == SUCESO_PEDIDO){
		llegapedido();
	} else if(nodo.suceso == SUCESO_FIN_SIMULACION){
		fin_simulacion();
	}
}

void Compania::demanda(){
	if(nivel > 0){
		acummas += (reloj-tultsuc)*nivel;
	} else {
		acummenos += (reloj-tultsuc)*(-nivel);
	}
	tultsuc = reloj;
	tam = genera_tamano();
	nivel -= tam;
	nodo.suceso = SUCESO_DEMANDA;
	nodo.tiempo = reloj+gendem(0.1);
	insertar_lsuc(nodo);
}

void Compania::evaluacion(){
	if(nivel < spequena && pedido == 0){
		pedido = sgrande - nivel;
		acumpedido += K+i*pedido;
		nodo.suceso = SUCESO_PEDIDO;
		nodo.tiempo = reloj+genpedido(0.5, 1.0);
		insertar_lsuc(nodo);
	}
	nodo.suceso = SUCESO_EVAL;
	nodo.tiempo = reloj+1.0;
	insertar_lsuc(nodo);
}

void Compania::llegapedido(){
	if(nivel > 0){
		acummas += (reloj-tultsuc)*nivel;
	} else {
		acummenos += (reloj-tultsuc)*(-nivel);
	}
	tultsuc = reloj;
	nivel += pedido;
	pedido = 0;
}

void Compania::fin_simulacion(){
	parar = true;
	ss.push_back(make_pair(sgrande,spequena));
	vector<double> acum {(acumpedido + acummas + acummenos)/reloj, acumpedido/reloj, acummas/reloj, acummenos/reloj};
	costes.push_back(acum);
}

void Compania::generador_informes(){
	double min_p = 0;
	double min = costes[0][0];
	cout << "Politica Costo Total Costo de pedido Costo de mantenimiento Costo de deficit" << endl;
	for(int i = 0; i < (int)costes.size(); i++){
		cout << "(" << ss[i].first << "," << ss[i].second << ")" << " ";
		if(min > costes[i][0]){
			min_p = i;
			min = costes[i][0];
		}
		for(int j = 0; j < (int)costes[i].size(); j++){
			cout << costes[i][j] << " ";
		}
		cout << endl;
	}
	cout << endl;
	cout << "El minimo: "<< min << " -> configuracion (" << ss[min_p].first << "," << ss[min_p].second << ")" << endl;
}

void Compania::simular(){
	for(auto i = S.begin(); i != S.end(); ++i){
		for(auto j = s.begin(); j != s.end(); ++j){
			if( *i < *j ){
				sgrande = *i;
				spequena = *j;
				inicializacion();
				while(!parar){
					temporizacion();
					suceso();
				}
			}
		}
	}
	generador_informes(1);
}
