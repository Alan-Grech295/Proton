using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Source.Proton.Math
{
    public class Matrix3x3
    {
        public float[,] matrix = new float[3,3];

        internal Matrix3x3(float[,] matrix)
        {
            this.matrix = matrix;
        }
    }
}
