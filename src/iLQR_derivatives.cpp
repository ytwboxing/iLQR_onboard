#include "iLQR.h"

#define eps 1e-6 // For finite differencing

// Given a trajectory {x(t),u(t)} from forward pass, compute deriviatives along it
void iLQR::compute_derivatives(const VecOfVecXd &x, const VecOfVecXd &u, VecOfMatXd &fx,
												 VecOfMatXd &fu, VecOfVecXd &cx, VecOfVecXd &cu,
												 VecOfMatXd &cxx, VecOfMatXd &cxu, VecOfMatXd &cuu)
{
  for (int t=0; t<T; t++)
  {
    get_dynamics_derivatives(x[t], u[t], fx[t], fu[t]);
    get_cost_derivatives(x[t], u[t], cx[t], cu[t]);
    get_cost_2nd_derivatives(x[t], u[t], cxx[t], cxu[t], cuu[t]);
    // std::cout << t << '\n';
    // std::cout << "fx[t]: \n" << fx[t] << '\n';
    // std::cout << "fu[t]: \n" << fu[t] << '\n';
    // std::cout << "cx[t]: \n" << cx[t] << '\n';
    // std::cout << "cu[t]: \n" << cu[t] << '\n';
    // std::cout << "cxx[t]: \n" << cxx[t] << '\n';
    // std::cout << "cxu[t]: \n" << cxu[t] << '\n';
    // std::cout << "cuu[t]: \n" << cuu[t] << '\n';
    // getchar();
  }
}


// -----------------
// Derivative computation

void iLQR::get_dynamics_derivatives(const VecXd &x, const VecXd &u,
															MatXd &fx, MatXd &fu)
{
  // TODO put these somewhere else? only needs to be done once
  fx.resize(n,n);
  fu.resize(n,m);

  VecXd plus, minus;
  for (int i=0; i<n; i++)
  {
    plus = minus = x;

    plus(i) += eps;
    minus(i) -= eps;

    fx.col(i) = (integrate_dynamics(plus, u)-integrate_dynamics(minus, u)) / (2*eps);
  }

  for (int i=0; i<m; i++){
    plus = minus = u;
    plus(i) += eps;
    minus(i) -= eps;
    fu.col(i) = (integrate_dynamics(x, plus)-integrate_dynamics(x, minus)) / (2*eps);
  }
}

void iLQR::get_cost_derivatives(const VecXd &x, const VecXd &u,
															VecXd &cx, VecXd &cu)
{
  cx.resize(n);
  cu.resize(m);

  VecXd plus, minus;
  for (int i=0; i<n; i++)
  {
    plus = minus = x;
    plus(i) += eps;
    minus(i) -= eps;
    cx(i) = (cost(plus, u)-cost(minus, u)) / (2*eps);
  }

  for (int i=0; i<m; i++){
    plus = minus = u;
    plus(i) += eps;
    minus(i) -= eps;
    cu(i) = (cost(x, plus)-cost(x, minus)) / (2*eps);
  }
}

void iLQR::get_cost_2nd_derivatives(const VecXd &x, const VecXd &u,
															MatXd &cxx, MatXd &cxu, MatXd &cuu)
{
  cxx.resize(n,n);
  cxu.resize(n,m);
  cuu.resize(m,m);

  //TODO remove repetition

  VecXd pp, pm, mp, mm; //plus-plus, plus-minus, ....

  //cxx
  for (int i=0; i<n; i++){
    for (int j=i; j<n; j++){
      pp = pm = mp = mm = x;
      pp(i) = pm(i) += eps;
      mp(i) = mm(i) -= eps;
      pp(j) = mp(j) += eps;
      pm(j) = mm(j) -= eps;
      cxx(i,j) = cxx(j,i) = (cost(pp, u) - cost(mp, u) - cost(pm, u) + cost(mm, u)) / (4*sqr(eps));
    }
  }
  //cxu
  for (int i=0; i<n; i++){
    for (int j=0; j<m; j++){
      pp = pm = mp = mm = x;
      pp(i) = pm(i) += eps;
      mp(i) = mm(i) -= eps;
      pp(j) = mp(j) += eps;
      pm(j) = mm(j) -= eps;
      cxu(i,j) = (cost(pp, u) - cost(mp, u) - cost(pm, u) + cost(mm, u)) / (4*sqr(eps));
    }
  }
  //cuu
  for (int i=0; i<m; i++){
    for (int j=i; j<m; j++){
      pp = pm = mp = mm = x;
      pp(i) = pm(i) += eps;
      mp(i) = mm(i) -= eps;
      pp(j) = mp(j) += eps;
      pm(j) = mm(j) -= eps;
      cuu(i,j) = cuu(j,i) = (cost(pp, u) - cost(mp, u) - cost(pm, u) + cost(mm, u)) / (4*sqr(eps));
    }
  }
} //get_cost_2nd_derivatives