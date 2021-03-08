import matplotlib.pyplot as plt
import pandas as pd
import argparse
import os
import sys

parser = argparse.ArgumentParser()

parser.add_argument('-i', '--input', dest='input', help='input file', type=str)
parser.add_argument('-o', '--output', dest='output', help='output file', type=str)

options = parser.parse_args()

if options.input is None:
    parser.print_usage()
    sys.exit()


data = pd.read_csv(options.input, delimiter=';')

print(data.columns[0])

# Uses the first column for the x axes
ax = data.plot(x=data.columns[0], marker='o', xticks=data.iloc[:,0])

# Set the bottom value to 0 for the Y axes
ax.set_ylim(bottom=0)

ax.set_xlabel('Problem size', fontsize='x-large')
ax.set_ylabel('Execution Time', fontsize='x-large')

# setting font sizes
ax.legend(fontsize='x-large')
plt.yticks(fontsize='x-large')
plt.xticks(fontsize='x-large')

# To have a graph that can be easily included in another document
plt.tight_layout(pad=0.5)


# filename for the output
if options.output is None:
    prefix, ext = os.path.splitext(options.input)
    outname = prefix + '.pdf'
else:
    outname = options.output

plt.savefig(outname, format='pdf', dpi=1200)

plt.show()



