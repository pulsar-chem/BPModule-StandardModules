import bpmodule as bp
from subprocess import call
from CallPsi4 import CallPsi4

class SCF(bp.modulebase.EnergyMethod):
  def __init__(self, myid):
    super(SCF, self).__init__(myid)

  def Deriv_(self,order):
      return CallPsi4(self,"SCF",order)