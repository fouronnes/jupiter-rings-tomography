#include <iostream>
#include <fstream>
#include "SpiceUsr.h"

#define JUNO_ASC_CHUA -61122
#define JUNO_ASC_CHUB -61121
#define JUNO_ASC_CHUC -61112
#define JUNO_ASC_CHUD -61111

void convert_date(SpiceDouble et) {
    SpiceChar utcstr[21];
    et2utc_c(et, "C", 1, 21, utcstr);
    std::cout << utcstr << std::endl;
}

void output_state(std::ostream& str, SpiceDouble et, SpiceDouble et1, SpiceDouble step) {
    SpiceDouble   state[6];
    SpiceDouble   lt;

    while (et <= et1) {
        et += step;

        // Get Juno state vector
        spkezr_c("JUNO_SPACECRAFT", et, "J2000", "NONE", "Jupiter", state, &lt);

        // Output time, x, y, z
        str << et << " " << state[0] << " " << state[1] << " " << state[2] << std::endl;
    }
}

void output_attitude(std::ostream& str, SpiceDouble et, SpiceDouble et1, SpiceDouble step) {
    while (et < et1) {
        et += step;

        // Convert time to spacecraft clock
        SpiceDouble spacecraft_time;
        sce2c_c(-61, et, &spacecraft_time);

        SpiceDouble mat[3][3];
        SpiceDouble clkout;
        SpiceBoolean found;

        // Get pointing attitude
        ckgp_c(JUNO_ASC_CHUA, spacecraft_time, 0, "J2000", mat, &clkout, &found);
        
        // Convert to equatorial coordinates
        SpiceDouble ang[3];
        m2eul_c(mat, 3, 1, 3, ang+2, ang+1, ang);
        
        SpiceDouble twist =  ang[2];
        SpiceDouble dec   =  halfpi_c() - ang[1];
        SpiceDouble ra    =  ang[0]     - halfpi_c();

        // Output RA, DE, twist
        str << ra << " " << dec << " " << twist << std::endl;
    }
}

void write_state(const char* name, SpiceDouble et, SpiceDouble et1, SpiceDouble step) {
    std::ofstream file(name);
    file.precision(6);
    file << std::fixed;
    output_state(file, et, et1, step);
}

int main()
{
    std::cout << "Loading kernels..." << std::endl;
    furnsh_c("setup.ker");
    SpiceDouble et, et1;

    // Orbit 1
    std::cout << "Orbit 1" << std::endl;
    str2et_c("2016 OCT 25 20:38:52", &et);
    str2et_c("2016 NOV 05 20:15:32", &et1);
    write_state("juno-state-orbit1.txt", et, et1, 60);

    // Orbit 2
    std::cout << "Orbit 2" << std::endl;
    str2et_c("2016 NOV 05 20:15:32", &et);
    str2et_c("2016 NOV 16 20:25:32", &et1);
    write_state("juno-state-orbit2.txt", et, et1, 60);

    // Orbit 3
    std::cout << "Orbit 3" << std::endl;
    str2et_c("2016 NOV 16 20:25:32", &et);
    str2et_c("2016 NOV 27 20:18:52", &et1);
    write_state("juno-state-orbit3.txt", et, et1, 60);

    // Orbit 4
    std::cout << "Orbit 4" << std::endl;
    str2et_c("2016 NOV 27 20:18:52", &et);
    str2et_c("2016 DEC 08 20:12:12", &et1);
    write_state("juno-state-orbit4.txt", et, et1, 60);

    // Attitude every half second
    std::cout << "Attitude" << std::endl;
    str2et_c("2016 NOV 27 20:10:00", &et);
    str2et_c("2016 NOV 27 20:15:00", &et1);
    std::ofstream attitude_file("juno-attitude.txt");
    attitude_file.precision(6);
    attitude_file << std::fixed;
    output_attitude(attitude_file, et, et1, 0.5);
}
