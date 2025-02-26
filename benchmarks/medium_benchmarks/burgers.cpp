#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>

using namespace std;

const int NX = 101;        // Number of spatial points
const double L = 2.0;      // Domain length
const double DX = L / (NX - 1); // Grid spacing
const double DT = 0.001;   // Time step
const double T_MAX = 0.05;  // Total simulation time
const double NU = 0.1;     // Viscosity (diffusion coefficient)

// Function to initialize u with a sinusoidal profile
void initialize(vector<double>& u) {
  for (int i = 0; i < NX; i++) {
    double x = i * DX;
    u[i] = sin(M_PI * x);  // Initial sine wave
  }
}

// Function to update u using Lax-Friedrichs scheme
void update(vector<double>& u, vector<double>& u_new) {
  for (int i = 1; i < NX - 1; i++) {
    // Convection term (nonlinear)
    double convection = -0.5 * (u[i + 1] * u[i + 1] - u[i - 1] * u[i - 1]) / DX;

    // Diffusion term (viscosity)
    double diffusion = NU * (u[i + 1] - 2 * u[i] + u[i - 1]) / (DX * DX);

    // Update step using Lax-Friedrichs
    u_new[i] = 0.5 * (u[i + 1] + u[i - 1]) + DT * (convection + diffusion);
  }

  // Periodic boundary conditions
  u_new[0] = u_new[NX - 2];
  u_new[NX - 1] = u_new[1];

  // Update u
  u = u_new;
}

// Function to save results to file for visualization
void save_to_file(const vector<double>& u, int timestep) {
  ofstream file("burgers_" + to_string(timestep) + ".dat");
  for (int i = 0; i < NX; i++) {
    file << i * DX << " " << u[i] << "\n";
  }
  file.close();
}

int main() {
  vector<double> u(NX, 0.0), u_new(NX, 0.0);
  initialize(u);

  int timestep = 0;
  double t = 0.0;
  while (t < T_MAX) {
    update(u, u_new);
    if (timestep % 2 == 0) { // Save every 100 steps
      save_to_file(u, timestep);
    }
    t += DT;
    timestep++;
  }

  cout << "Simulation complete. Data saved for visualization.\n";
  return 0;
}
