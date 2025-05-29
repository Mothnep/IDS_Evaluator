# simple_roc_plot.py
import pandas as pd
import matplotlib.pyplot as plt
import os
import glob

# Set non-interactive backend to avoid display issues
import matplotlib
matplotlib.use('Agg')

def plot_roc_curve():
    """Find and plot ROC curves from CSV files in ROC_Data_Points directory."""
    # Find all ROC data files
    data_dir = 'ROC_Data_Points'
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
            
            # Save plot
            output_file = f"{algorithm}_{dataset}_roc.png"
            plt.savefig(output_file, dpi=300)
            print(f"ROC curve saved to {output_file}")
            
            plt.close()
            
        except Exception as e:
            print(f"Error processing {file}: {e}")

if __name__ == "__main__":
    plot_roc_curve()