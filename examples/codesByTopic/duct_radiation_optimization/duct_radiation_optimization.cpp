/*
O que quer que tenha que acontecer, aconteça!
Seja qual for a situação, está ótimo!
Eu realmente não preciso de coisa alguma!
*/
#include "palabos3D.h"
#include "palabos3D.hh"
#include <vector>
#include <cmath>
#include <time.h>
#include <cfloat>

using namespace plb;
using namespace plb::descriptors;
using namespace std;

typedef double T;
typedef Array<T,3> Velocity;
#define DESCRIPTOR MRTD3Q19Descriptor
typedef MRTdynamics<T,DESCRIPTOR> BackgroundDynamics;
typedef AnechoicMRTdynamics<T,DESCRIPTOR> AnechoicBackgroundDynamics;

// ---------------------------------------------
// Includes of acoustics resources
#include "acoustics/acoustics3D.h"
using namespace plb_acoustics_3D;
// ---------------------------------------------

int main(int argc, char **argv){
    plbInit(&argc, &argv);

    global::timer("mainLoop").start();

    const plint radius = atoi(argv[1]);
    pcout << atoi(argv[1]) << endl;
    const plint diameter = 2*radius;
    const T mach_number = atof(argv[2]);
    pcout << mach_number << endl;
    const T lattice_speed_sound = 1/sqrt(3);
    const T velocity_flow = mach_number*lattice_speed_sound;
    const T Re = atof(argv[3]);
    pcout << Re << endl;
    const T tau = (0.5 + ((velocity_flow*diameter)/(Re*lattice_speed_sound*lattice_speed_sound)));
    T omega = atof(argv[3]);

    const T rho0 = 1;
    const T drho = compute_drho(120); // NPS dB

    const plint nx = (6*diameter + 60);
    //const plint nx = 2*diameter + 60;

    const plint ny = (6*diameter + 60);
    //const plint ny = 2*diameter + 60;

    const plint position_duct_z = 0;
    //const plint length_duct = 0.5*(30 + 120 + 5*113 + 3*diameter);
    const plint length_duct = 3*(3*diameter);
    //const plint length_duct = 3*diameter;

    //const plint nz = length_duct + (60/2)*diameter + 30; // precious
    const plint nz = length_duct + 3*diameter + 30;

    //const plint transient_time = (nz)/(mach_number*lattice_speed_sound);
    const plint transient_time = 16000; 
    const plint maxT = 12000 + transient_time;
    Array<T,3> u0(0, 0, 0);
    const Array<plint,3> position(nx/2, ny/2, position_duct_z);
    const plint thickness_duct = 2;
    const plint radius_intern = radius - 2;
    const plint maxT_final_source = maxT - nz*sqrt(3);
    const T ka_max = 2.5;
    const T ka_min = 0;
    const T cs2 = lattice_speed_sound*lattice_speed_sound;

    // Saving a propery directory
    std::string fNameOut = currentDateTime() + "+tmp";
    std::string command = "mkdir -p " + fNameOut;
    char to_char_command[1024];
    strcpy(to_char_command, command.c_str());
    system(to_char_command);
    std::string command_copy_script_matlab = "cp duct_radiation.m " + fNameOut;
    char to_char_command_copy_script_matlab[1024];
    strcpy(to_char_command_copy_script_matlab, command_copy_script_matlab.c_str());
    system(to_char_command_copy_script_matlab);
    global::directories().setOutputDir(fNameOut+"/");


    // Setting anechoic dynamics like this way
    MultiBlockLattice3D<T, DESCRIPTOR> lattice(nx, ny, nz,  new AnechoicBackgroundDynamics(omega));
    defineDynamics(lattice, lattice.getBoundingBox(), new BackgroundDynamics(omega));

    pcout << "Creation of the lattice. Time max: " << maxT << endl;
    pcout << "Duct radius: " << radius << endl;

    pcout << getMultiBlockInfo(lattice) << std::endl;

    // Switch off periodicity.
    lattice.periodicity().toggleAll(false);

    pcout << "Initilization of rho and u." << endl;
    initializeAtEquilibrium(lattice, lattice.getBoundingBox(), rho0 , u0);

    // Set NoDynamics to improve performance!
    plint off_set_z = position_duct_z + length_duct - 3*diameter - 30;
    set_nodynamics(lattice, nx, ny, off_set_z);

    T rhoBar_target = 0;
    //const T mach_number = 0;
    Array<T,3> j_target(0, 0, 0);
    T size_anechoic_buffer = 30;
    defineAnechoicMRTBoards_limited(nx, ny, nz, lattice, size_anechoic_buffer,
      omega, j_target, j_target, j_target, j_target, j_target, j_target,
      rhoBar_target, off_set_z);

    //const T mach_number = 0;
    build_duct(lattice, nx, ny, position, radius, length_duct, thickness_duct, omega);

    lattice.initialize();

    pcout << std::endl << "Voxelizing the domain." << std::endl;

    pcout << "Simulation begins" << endl;

    // Setting probes ------------------------------------------
    plint begin_microphone = length_duct/2;
    System_Abom_Measurement system_abom_measurement(lattice, position,
        begin_microphone, length_duct, radius, fNameOut);
    plint xa = nx/2 - diameter; 
    plint xb = nx/2 + diameter;
    plint ya = ny/2; 
    plint yb = ny/2; 
    plint za = 0;
    plint zb = length_duct + diameter;
    Box3D plane(xa, xb, ya, yb, za, zb);
    plint max_time_period = atoi(argv[5]);
    plint initial_time = atoi(argv[6]);
    Howe_Corollary howe_corollary(plane, max_time_period, initial_time);
    pcout << initial_time << endl;
    pcout << max_time_period << endl;

    string name_point_1 = "point_1";
    Array<plint,2> centering_point_1(4, 4);
    System_Abom_Measurement_points system_abom_measurement_point_1(lattice, position,
        begin_microphone, length_duct, radius, fNameOut, centering_point_1, name_point_1);

    string name_point_2 = "point_2";
    Array<plint,2> centering_point_2(8, 8);
    System_Abom_Measurement_points system_abom_measurement_point_2(lattice, position,
        begin_microphone, length_duct, radius, fNameOut, centering_point_2, name_point_2);

    string name_point_3 = "point_3";
    Array<plint,2> centering_point_3(12, 12);
    System_Abom_Measurement_points system_abom_measurement_point_3(lattice, position,
        begin_microphone, length_duct, radius, fNameOut, centering_point_3, name_point_3);

    string name_point_4 = "point_4";
    Array<plint,2> centering_point_4(16, 16);
    System_Abom_Measurement_points system_abom_measurement_point_4(lattice, position,
        begin_microphone, length_duct, radius, fNameOut, centering_point_4, name_point_4);

    string name_point_5 = "point_5";
    Array<plint,2> centering_point_5(20, 20);
    System_Abom_Measurement_points system_abom_measurement_point_5(lattice, position,
        begin_microphone, length_duct, radius, fNameOut, centering_point_5, name_point_5);

    // Recording entering signal -------------------------------
    std::string signal_in_string = fNameOut+"/signal_in.dat";
    char to_char_signal_in[1024];
    strcpy(to_char_signal_in, signal_in_string.c_str());
    plb_ofstream history_signal_in(to_char_signal_in);
    // ---------------------------------------------------------

    // Important information about simulation ------------------
    std::string AllSimulationInfo_string = fNameOut + "/AllSimulationInfo.txt";
    char to_char_AllSimulationInfo[1024];
    strcpy(to_char_AllSimulationInfo, AllSimulationInfo_string.c_str());
    plb_ofstream AllSimulationInfo(to_char_AllSimulationInfo);

    std::string title = "\nTestando varios pontos para pegar a energia turbulenta +  colorario para Mach 0.07.\n";

    AllSimulationInfo << endl
    << title << endl
    << "Dados da simulação" << endl
    << "Lattice:" << endl << endl
    << "nx: " << nx << " ny: " << ny << " nz: " << nz << endl
    << "omega: " << omega << endl << endl
    << "Tempos: " << endl
    << "Total Time step: " << maxT << endl
    << "Raio do duto: " << radius << endl
    << "Espessura: " << thickness_duct << endl
    << "Tamanho duto: " << length_duct << endl
    << "Posicao do duto: " << position[2] << endl
    << "Começo dos microfones em lattice: " << begin_microphone << endl
    << "Mach imposto: " <<  mach_number << endl
    << "Dimensoes plano: " <<  xa << endl
    <<  xb << endl
    <<  ya << endl
    <<  yb << endl
    <<  za << endl
    <<  zb << endl
    << initial_time << endl
    << max_time_period << endl
    ;
    // --------------------------------------------------------

    // Mean for-loop
    for (plint iT=0; iT<maxT; ++iT){
        if (iT <= maxT_final_source && iT > transient_time){
            plint total_signals = 20;
            T ka = atof(argv[4]);
            T chirp_hand = get_tonal(ka, maxT_final_source, iT, drho, radius);
            history_signal_in << setprecision(10) << chirp_hand << endl;
            Array<T,3> j_target(0, 0, velocity_flow);
            set_source(lattice, position, chirp_hand, j_target, radius, radius_intern, nx, ny);
        
        }else{
            Array<T,3> j_target(0, 0, velocity_flow);
            set_source(lattice, position, rho0, j_target, radius, radius_intern, nx, ny);
        }

        if (iT % 50 == 0) {
            pcout << "Iteration " << iT << endl;
             pcout << " energy ="
            << setprecision(10) << getStoredAverageEnergy<T>(lattice)
            << " rho ="
            << getStoredAverageDensity<T>(lattice)
            << " max_velocity ="
            << setprecision(10) << (getStoredMaxVelocity<T>(lattice))/lattice_speed_sound
            << endl;
        }

        // extract values of pressure and velocities
        system_abom_measurement.save_point(lattice, rho0, cs2);
        if (iT >= howe_corollary.get_initial_time() && iT <= howe_corollary.get_total_period()){
            howe_corollary.extract_velocities(lattice);
        }

        system_abom_measurement_point_1.save_point(lattice, rho0, cs2);
        system_abom_measurement_point_2.save_point(lattice, rho0, cs2);
        system_abom_measurement_point_3.save_point(lattice, rho0, cs2);
        system_abom_measurement_point_4.save_point(lattice, rho0, cs2);
        system_abom_measurement_point_5.save_point(lattice, rho0, cs2);
        
        lattice.collideAndStream();
    }

    string test_file_name = "howe_corollary_"; 
    howe_corollary.calculate_acoustic_energy(fNameOut, test_file_name);

    T total_time_simulation = global::timer("mainLoop").stop();
    pcout << "End of simulation at iteration with total time: " << total_time_simulation << endl;
    AllSimulationInfo << endl << "Execution time: " << total_time_simulation << " segundos" << endl;

    pcout << "End of simulation at iteration " << endl;
}
