using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Proton
{
    public class Matrix3x3
    {
        public float[,] matrix = new float[3,3];

        public float this[int row, int column]
        {
            get
            {
                return matrix[row, column];
            }

            set
            {
                matrix[row, column] = value;
            }
        }

        internal Matrix3x3()
        {
            matrix = new float[3,3];
        }

        internal Matrix3x3(Vector3[] rows)
        {
            if (rows.Length != 3)
                throw new Exception("Rows need to be a length of 3");

            matrix = new float[3, 3];

            for(int i = 0; i < 3; i++)
            {
                Vector3 row = rows[i];
                matrix[i, 0] = row.x;
                matrix[i, 1] = row.y;
                matrix[i, 2] = row.z;
            }
        }
    }
}
