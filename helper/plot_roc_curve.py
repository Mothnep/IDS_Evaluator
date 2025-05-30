# simple_roc_plot.py
import pandas as pd
import matplotlib.pyplot as plt
import os
import glob
import sys

# Set non-interactive backend to avoid display issues
import matplotlib
matplotlib.use('Agg')

def plot_roc_curve(output_dir=None):
    """Find and plot ROC curves from CSV files in ROC_CSV directory.
    
    Args:
        output_dir: Directory to save the plots to. If None, saves to current directory.
    """
    # Find all ROC data files
    data_dir = 'algorithms/ROC_CSV'  # Path relative to project root
    if not os.path.exists(data_dir):
        print(f"Directory {data_dir} not found")
        return
    
    files = glob.glob(os.path.join(data_dir, '*_roc.csv'))
    if not files:
        print("No ROC data files found")
        return
    
    # Process each file
    for file in files:
        try:
            # Load data
            df = pd.read_csv(file)
            
            # Get algorithm and dataset names from filename
            base = os.path.basename(file)
            name_parts = base.replace('_roc.csv', '').split('_')
            algorithm = name_parts[0]
            dataset = name_parts[1] if len(name_parts) > 1 else "Unknown"
            
            # Create plot
            plt.figure(figsize=(8, 6))
            
            # Plot ROC curve
            plt.plot(df['fpr'], df['tpr'], 'b-', linewidth=2)
            
            # Calculate AUC using trapezoidal rule
            auc = 0.0
            fpr = df['fpr'].values
            tpr = df['tpr'].values
            for i in range(1, len(fpr)):
                auc += (fpr[i] - fpr[i-1]) * (tpr[i] + tpr[i-1]) / 2.0
            
            # Plot random classifier reference line
            plt.plot([0, 1], [0, 1], 'k--', alpha=0.5)
            
            # Simple styling
            plt.xlim([0, 1])
            plt.ylim([0, 1])
            plt.xlabel('False Positive Rate')
            plt.ylabel('True Positive Rate')
            plt.title(f'ROC Curve - {algorithm} on {dataset}\nAUC = {auc:.4f}')
            plt.grid(True, alpha=0.3)
            
            # Save plot to specified directory or current dir
            output_file = f"{algorithm}_{dataset}_roc.png"
            if output_dir:
                output_path = os.path.join(output_dir, output_file)
                plt.savefig(output_path, dpi=300)
                print(f"ROC curve saved to {output_path}")
            else:
                plt.savefig(output_file, dpi=300)
                print(f"ROC curve saved to {output_file}")
            
            plt.close()
            
        except Exception as e:
            print(f"Error processing {file}: {e}")

if __name__ == "__main__":
    # Check if output directory was provided as argument
    output_dir = sys.argv[1] if len(sys.argv) > 1 else None
    plot_roc_curve(output_dir)