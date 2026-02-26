use clap::Parser;
use std::fs;
use std::io::Write;

#[derive(Parser, Debug)]
#[command(name = "gen_lorentz")]
#[command(about = "Generate a C conjugate gradient solver program from matrix file", long_about = None)]
struct Args {
    /// Matrix file name (format: row:col:value for A, then row:value for b)
    #[arg(short, long)]
    matrix_file: String,

    /// Matrix size (N x N)
    #[arg(short = 'n', long)]
    size: usize,

    /// Output tag name
    #[arg(short, long)]
    tag: String,

    /// Input error value
    #[arg(short, long, default_value_t = 0.0)]
    error: f64,

    /// Number of iterations
    #[arg(short = 'k', long)]
    iterations: usize,

    /// Output directory
    #[arg(short, long, default_value = "scratch")]
    output_dir: String,
}

fn main() {
    let args = Args::parse();

    let n = args.size;
    let max_k = args.iterations;
    let tagname = args.tag;
    let err = args.error;
    let output_dir = args.output_dir;

    // Ensure output directory exists
    fs::create_dir_all(&output_dir).expect("Failed to create output directory");

    // Read the A matrix and b vector from file
    let mut a: Vec<Vec<f64>> = vec![vec![0.0; n]; n];
    let mut b: Vec<f64> = vec![0.0; n];

    let contents = fs::read_to_string(&args.matrix_file)
        .expect("Failed to read matrix file");
    let lines: Vec<&str> = contents.lines().collect();

    // Read A matrix (first N*N lines)
    for i in 0..(n * n) {
        let parts: Vec<&str> = lines[i].split(':').collect();
        if parts.len() != 3 {
            panic!("Invalid matrix format at line {}: expected row:col:value", i);
        }
        let row: usize = parts[0].parse().expect("Invalid row index");
        let col: usize = parts[1].parse().expect("Invalid col index");
        let val: f64 = parts[2].parse().expect("Invalid matrix value");
        a[row][col] = val;
    }

    // Read b vector (next N lines)
    for i in (n * n)..(n * n + n) {
        let parts: Vec<&str> = lines[i].split(':').collect();
        if parts.len() != 2 {
            panic!("Invalid vector format at line {}: expected row:value", i);
        }
        let row: usize = parts[0].parse().expect("Invalid row index");
        let val: f64 = parts[1].parse().expect("Invalid vector value");
        b[row] = val;
    }

    let mut dump_str = String::new();

    // Initialize R and P arrays
    let mut r: Vec<Vec<String>> = vec![vec![String::new(); n], vec![String::new(); n]];
    let mut p: Vec<Vec<String>> = vec![vec![String::new(); n], vec![String::new(); n]];

    // Compute initial residual r_0 = b - A*x_0
    let k = 0;
    for i in 0..n {
        let rhs_matvec_i = format!(
            "b_{} - {}",
            i,
            (0..n)
                .map(|j| format!("A_{}_{} * x_{}_{}", i, j, k, j))
                .collect::<Vec<_>>()
                .join(" + ")
        );
        let lhs_matvec_i = format!("r_{}_{}", k, i);
        r[0][i] = lhs_matvec_i.clone();
        dump_str.push_str(&format!("\t\tdouble {} = {};\n", lhs_matvec_i, rhs_matvec_i));
    }

    // Initialize p_0 = r_0
    p[0] = r[0].clone();

    // Main CG iteration loop
    for k in 0..max_k {
        // Compute r^T * r
        let rhs_rtr = (0..n)
            .map(|i| format!("{} * {}", r[0][i], r[0][i]))
            .collect::<Vec<_>>()
            .join(" + ");
        let lhs_rtr = format!("rtr_{}", k);
        dump_str.push_str(&format!("\t\tdouble {} = {};\n", lhs_rtr, rhs_rtr));

        // Compute A*p
        let mut rhs_ap = vec![String::new(); n];
        for i in 0..n {
            let rhs_ap_i = (0..n)
                .map(|j| format!("A_{}_{} * {}", i, j, p[0][j]))
                .collect::<Vec<_>>()
                .join(" + ");
            let lhs_ap_i = format!("AP_{}_{}", k, i);
            rhs_ap[i] = lhs_ap_i.clone();
            dump_str.push_str(&format!("\t\tdouble {} = {};\n", lhs_ap_i, rhs_ap_i));
        }

        // Compute p^T * A * p
        let rhs_pap = (0..n)
            .map(|i| format!("{} * {}", p[0][i], rhs_ap[i]))
            .collect::<Vec<_>>()
            .join(" + ");
        let lhs_pap = format!("pap_{}", k);
        dump_str.push_str(&format!("\t\tdouble {} = {};\n", lhs_pap, rhs_pap));

        // Compute alpha = (r^T * r) / (p^T * A * p)
        let rhs_alpha = format!("({}) / ({})", lhs_rtr, lhs_pap);
        let lhs_alpha = format!("alpha_{}", k);
        dump_str.push_str(&format!("\t\tdouble {} = {};\n", lhs_alpha, rhs_alpha));

        // Update x: x_{k+1} = x_k + alpha * p_k
        for i in 0..n {
            let rhs_x = format!("x_{}_{} + {} * {}", k, i, lhs_alpha, p[0][i]);
            let lhs_x = format!("x_{}_{}", k + 1, i);
            dump_str.push_str(&format!("\t\tdouble {} = {};\n", lhs_x, rhs_x));
        }

        // Update r: r_{k+1} = r_k - alpha * A * p_k
        for i in 0..n {
            let rhs_next_residue = format!("{} - {} * {}", r[0][i], lhs_alpha, rhs_ap[i]);
            let lhs_next_residue = format!("r_{}_{}", k + 1, i);
            r[1][i] = lhs_next_residue.clone();
            dump_str.push_str(&format!("\t\tdouble {} = {};\n", lhs_next_residue, rhs_next_residue));
        }

        // Compute r_{k+1}^T * r_{k+1}
        let rhs_r1tr1 = (0..n)
            .map(|i| format!("{} * {}", r[1][i], r[1][i]))
            .collect::<Vec<_>>()
            .join(" + ");
        let lhs_r1tr1 = format!("r1tr1_{}", k);
        dump_str.push_str(&format!("\t\tdouble {} = {};\n", lhs_r1tr1, rhs_r1tr1));

        // Compute r_k^T * r_k
        let rhs_r0tr0 = (0..n)
            .map(|i| format!("{} * {}", r[0][i], r[0][i]))
            .collect::<Vec<_>>()
            .join(" + ");
        let lhs_r0tr0 = format!("r0tr0_{}", k);
        dump_str.push_str(&format!("\t\tdouble {} = {};\n", lhs_r0tr0, rhs_r0tr0));

        // Compute beta = (r_{k+1}^T * r_{k+1}) / (r_k^T * r_k)
        let rhs_beta = format!("({}) / ({})", lhs_r1tr1, lhs_r0tr0);
        let lhs_beta = format!("beta_{}", k);
        dump_str.push_str(&format!("\t\tdouble {} = {};\n", lhs_beta, rhs_beta));

        // Update p: p_{k+1} = r_{k+1} + beta * p_k
        for i in 0..n {
            let rhs_next_p = format!("{} + {} * {}", r[1][i], lhs_beta, p[0][i]);
            let lhs_next_p = format!("p_{}_{}", k + 1, i);
            p[1][i] = lhs_next_p.clone();
            dump_str.push_str(&format!("\t\tdouble {} = {};\n", lhs_next_p, rhs_next_p));
        }

        // Swap buffers
        p[0] = p[1].clone();
        r[0] = r[1].clone();
    }

    // Generate output C file
    let outfile = format!("CG_{}_K{}_N{}.c", tagname, max_k, n);
    let outpath = format!("{}/{}", output_dir, outfile);

    let mut fout = fs::File::create(&outpath).expect("Failed to create output file");

    // Write includes
    fout.write_all(b"#include <stdio.h>\n").unwrap();
    fout.write_all(b"#include <math.h>\n\n").unwrap();

    // Write function signature
    fout.write_all(b"double src(\n").unwrap();
    for i in 0..n {
        fout.write_all(b"\t").unwrap();
        for j in 0..n {
            write!(fout, "double A_{}_{},", i, j).unwrap();
        }
        fout.write_all(b"\n").unwrap();
    }
    fout.write_all(b"\t").unwrap();
    for i in 0..n {
        write!(fout, "double b_{},", i).unwrap();
    }
    fout.write_all(b"\n\t").unwrap();
    for i in 0..n - 1 {
        write!(fout, "double x_0_{},", i).unwrap();
    }
    write!(fout, "double x_0_{}", n - 1).unwrap();
    fout.write_all(b") {\n").unwrap();

    // Write the computation
    fout.write_all(dump_str.as_bytes()).unwrap();
    write!(fout, "\t\treturn x_{}_{};\n", max_k, n - 1).unwrap();
    fout.write_all(b"}\n\n").unwrap();

    // Write main function with values from matrix file
    fout.write_all(b"int main() {\n").unwrap();
    fout.write_all(b"\t// Initialize the input values\n").unwrap();

    // Initialize b vector
    for i in 0..n {
        write!(fout, "\tdouble b_{} = {};\n", i, b[i]).unwrap();
    }

    // Initialize A matrix
    for i in 0..n {
        for j in 0..n {
            write!(fout, "\tdouble A_{}_{} = {};\n", i, j, a[i][j]).unwrap();
        }
    }

    // Initialize x vector
    for i in 0..n {
        write!(fout, "\tdouble x_0_{} = 0.0;\n", i).unwrap();
    }

    fout.write_all(b"\n\t// Call the src function\n").unwrap();
    fout.write_all(b"\tdouble result = src(\n").unwrap();
    for i in 0..n {
        fout.write_all(b"\t\t").unwrap();
        for j in 0..n {
            write!(fout, "A_{}_{}, ", i, j).unwrap();
        }
        fout.write_all(b"\n").unwrap();
    }
    fout.write_all(b"\t\t").unwrap();
    for i in 0..n {
        write!(fout, "b_{}, ", i).unwrap();
    }
    fout.write_all(b"\n\t\t").unwrap();
    for i in 0..n - 1 {
        write!(fout, "x_0_{}, ", i).unwrap();
    }
    write!(fout, "x_0_{});\n", n - 1).unwrap();

    fout.write_all(b"\n\t// Print the result\n").unwrap();
    fout.write_all(b"\tprintf(\"%f\\n\", result);\n").unwrap();
    fout.write_all(b"\n\treturn 0;\n").unwrap();
    fout.write_all(b"}\n").unwrap();

    println!("Generated C conjugate gradient solver: {}", outpath);
    println!("Matrix file: {}", args.matrix_file);
    println!("Matrix size: {}x{}", n, n);
    println!("Iterations: {}", max_k);
    println!("Error value: {}", err);
}
