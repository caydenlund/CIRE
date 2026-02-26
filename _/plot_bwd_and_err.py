import plotly.express as px
import pandas as pd

# Function to read data from CSV and plot
def plot_bwd_local_error(csv_filename):
    # Read the data file
    df = pd.read_csv(csv_filename)

    print(df)

    # Ensure the required columns exist
    # required_columns = {'Node Id', 'Depth', 'Bwd', 'Local Error'}
    # if not required_columns.issubset(df.columns):
    #     print(f"Error: CSV file must contain columns {required_columns}")
    #     return

    # Plot Bwd and Local Error against Node Id
    fig1 = px.scatter(df, x='Node Id', y=df.columns[2:],
                      title="Bwd and Local Error vs Node Id",
                      labels={'value': 'Bwd and Local Error', 'variable': 'Metric'},
                      symbol_sequence=['circle', 'square'])

    # Plot Bwd and Local Error against Depth
    fig2 = px.scatter(df, x='Depth', y=df.columns[2:],
                      title="Bwd and Local Error vs Depth",
                      labels={'value': 'Bwd and Local Error', 'variable': 'Metric'},
                      symbol_sequence=['circle', 'square'])

    # Show the plots
    fig1.show()
    fig2.show()


def main():
    import sys
    if len(sys.argv) != 2:
        print("Usage: python plot_bwd_and_err.py <filename>")
        return
    filename = sys.argv[1]
    plot_bwd_local_error(filename)


if __name__ == '__main__':
    main()