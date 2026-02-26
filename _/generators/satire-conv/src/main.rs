use anyhow::{Context, Result};
use clap::Parser;
use regex::Regex;
use std::fs;
use std::path::PathBuf;

/// Satire to C code generator
#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    /// Input Satire program file (.txt)
    input: PathBuf,

    /// Output C code file
    #[arg(short, long)]
    output: PathBuf,
}

#[derive(Debug, Clone)]
struct Input {
    name: String,
    typ: String,
    lower: f64,
    upper: f64,
}

#[derive(Debug, Clone)]
struct Expression {
    var: String,
    typ: String,
    rhs: String,
}

#[derive(Debug)]
struct SatireProgram {
    inputs: Vec<Input>,
    outputs: Vec<String>,
    expressions: Vec<Expression>,
}

impl SatireProgram {
    fn parse(content: &str) -> Result<Self> {
        let inputs = Self::parse_inputs(content)?;
        let outputs = Self::parse_outputs(content)?;
        let expressions = Self::parse_expressions(content)?;

        Ok(SatireProgram {
            inputs,
            outputs,
            expressions,
        })
    }

    fn parse_inputs(content: &str) -> Result<Vec<Input>> {
        let mut inputs = Vec::new();

        // Find INPUTS section
        let inputs_re = Regex::new(r"(?s)INPUTS\s*\{(.*?)\}").unwrap();
        let inputs_section = inputs_re
            .captures(content)
            .context("Could not find INPUTS section")?
            .get(1)
            .unwrap()
            .as_str();

        // Parse each input declaration: name type : (lower, upper);
        let input_re =
            Regex::new(r"(\w+)\s+(fl64|fl32|fl16|int)\s*:\s*\(\s*([^,]+)\s*,\s*([^)]+)\s*\)\s*;")
                .unwrap();

        for cap in input_re.captures_iter(inputs_section) {
            let name = cap[1].to_string();
            let typ = cap[2].to_string();
            let lower_str = cap[3].trim();
            let upper_str = cap[4].trim();

            // Parse bounds as f64
            let lower: f64 = lower_str
                .parse()
                .with_context(|| format!("Failed to parse lower bound: {lower_str}"))?;
            let upper: f64 = upper_str
                .parse()
                .with_context(|| format!("Failed to parse upper bound: {upper_str}"))?;

            inputs.push(Input {
                name,
                typ,
                lower,
                upper,
            });
        }

        if inputs.is_empty() {
            anyhow::bail!("No inputs found in INPUTS section");
        }

        Ok(inputs)
    }

    fn parse_outputs(content: &str) -> Result<Vec<String>> {
        let mut outputs = Vec::new();

        // Find OUTPUTS section
        let outputs_re = Regex::new(r"(?s)OUTPUTS\s*\{(.*?)\}").unwrap();
        let outputs_section = outputs_re
            .captures(content)
            .context("Could not find OUTPUTS section")?
            .get(1)
            .unwrap()
            .as_str();

        // Parse each output: name;
        let output_re = Regex::new(r"(\w+)\s*;").unwrap();

        for cap in output_re.captures_iter(outputs_section) {
            outputs.push(cap[1].to_string());
        }

        if outputs.is_empty() {
            anyhow::bail!("No outputs found in OUTPUTS section");
        }

        Ok(outputs)
    }

    fn parse_expressions(content: &str) -> Result<Vec<Expression>> {
        let mut expressions = Vec::new();

        // Find EXPRS section
        let exprs_re = Regex::new(r"(?s)EXPRS\s*\{(.*?)\}").unwrap();
        let exprs_section = exprs_re
            .captures(content)
            .context("Could not find EXPRS section")?
            .get(1)
            .unwrap()
            .as_str();

        // Parse each expression: var type = rhs;
        let expr_re = Regex::new(r"(\w+)\s+(fl64|fl32|fl16|int)\s*=\s*(.+?)\s*;").unwrap();

        for cap in expr_re.captures_iter(exprs_section) {
            let var = cap[1].to_string();
            let typ = cap[2].to_string();
            let rhs = cap[3].trim().to_string();

            expressions.push(Expression { var, typ, rhs });
        }

        if expressions.is_empty() {
            anyhow::bail!("No expressions found in EXPRS section");
        }

        Ok(expressions)
    }

    fn generate_c_code(&self) -> String {
        let mut output = String::new();

        // Add header comment
        output.push_str("/**\n");
        output.push_str(" * Generated from Satire program\n");
        output.push_str(" *\n");
        output.push_str(" * Inputs:\n");
        for input in &self.inputs {
            output.push_str(&format!(
                " *   {} ({}) : [{}, {}]\n",
                input.name,
                Self::satire_type_to_c(&input.typ),
                input.lower,
                input.upper
            ));
        }
        output.push_str(" *\n");
        output.push_str(" * Outputs:\n");
        for out in &self.outputs {
            output.push_str(&format!(" *   {out}\n"));
        }
        output.push_str(" */\n\n");

        // Add necessary includes
        output.push_str("#include <math.h>\n\n");

        // Generate function signature
        let return_type = if let Some(last_expr) = self.expressions.last() {
            Self::satire_type_to_c(&last_expr.typ)
        } else {
            "void"
        };

        output.push_str(&format!("{return_type} compute("));

        // Add parameters
        let params: Vec<String> = self
            .inputs
            .iter()
            .map(|input| format!("{} {}", Self::satire_type_to_c(&input.typ), input.name))
            .collect();
        output.push_str(&params.join(", "));
        output.push_str(") {\n");

        // Generate variable declarations and assignments
        for expr in &self.expressions {
            let c_type = Self::satire_type_to_c(&expr.typ);
            output.push_str(&format!("    {} {} = {};\n", c_type, expr.var, expr.rhs));
        }

        // Return the last computed variable (which should be in outputs)
        if let Some(last_expr) = self.expressions.last() {
            output.push_str(&format!("    return {};\n", last_expr.var));
        }

        output.push_str("}\n");

        output
    }

    fn satire_type_to_c(satire_type: &str) -> &str {
        match satire_type {
            "fl64" => "double",
            "fl32" => "float",
            "fl16" => "_Float16",
            "int" => "int",
            _ => "double", // default fallback
        }
    }
}

fn main() -> Result<()> {
    let args = Args::parse();

    // Read input file
    let content = fs::read_to_string(&args.input)
        .with_context(|| format!("Failed to read input file: {:?}", args.input))?;

    // Parse Satire program
    let program = SatireProgram::parse(&content)
        .with_context(|| format!("Failed to parse Satire program: {:?}", args.input))?;

    // Generate C code
    let c_code = program.generate_c_code();

    // Write output file
    fs::write(&args.output, c_code)
        .with_context(|| format!("Failed to write output file: {:?}", args.output))?;

    println!("Successfully generated C code: {:?}", args.output);

    Ok(())
}
