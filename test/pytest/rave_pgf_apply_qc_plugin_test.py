'''
Copyright (C) 2010 Swedish Meteorological and Hydrological Institute, SMHI,

This file is part of RAVE.

RAVE is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RAVE is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with RAVE.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------*/

Tests the PGF plugin for applying Quality Controls to an existing Polar Volume

@file
@author Mats Vernersson (Swedish Meteorological and Hydrological Institute, SMHI)
@date 2016-04-15
'''
import unittest
import rave_pgf_apply_qc_plugin
import rave_quality_plugin, rave_pgf_quality_registry
import mock
import rave_overshooting_quality_plugin

class rave_pgf_apply_qc_plugin_test(unittest.TestCase):
  def setUp(self):
    self.qc_check_1_mock = mock.Mock(spec=rave_quality_plugin.rave_quality_plugin)
    self.qc_check_2_mock = mock.Mock(spec=rave_quality_plugin.rave_quality_plugin)
    rave_pgf_quality_registry.add_plugin("qc.check.1", self.qc_check_1_mock)
    rave_pgf_quality_registry.add_plugin("qc.check.2", self.qc_check_2_mock)

  def tearDown(self):
    rave_pgf_quality_registry.remove_plugin("qc.check.1")
    rave_pgf_quality_registry.remove_plugin("qc.check.2")
    
  def test_perform_quality_control(self):
    vol = object()
    
    self.qc_check_1_mock.process.return_value = vol
    self.qc_check_2_mock.process.return_value = vol

    result = rave_pgf_apply_qc_plugin.perform_quality_control(vol, ["qc.check.1","qc.check.2"])
    
    expected_qc_check_1_calls = [mock.call.process(vol)]
    expected_qc_check_2_calls = [mock.call.process(vol)]
    
    self.assertTrue(expected_qc_check_1_calls == self.qc_check_1_mock.mock_calls)
    self.assertTrue(expected_qc_check_2_calls == self.qc_check_2_mock.mock_calls)
    self.assertTrue(vol == result)

  def test_perform_quality_control_process_return_tuple(self):
    vol = object()
    a1 = object()
    
    self.qc_check_1_mock.process.return_value = vol
    self.qc_check_2_mock.process.return_value = (vol,a1)

    result = rave_pgf_apply_qc_plugin.perform_quality_control(vol, ["qc.check.1","qc.check.2"])
    
    expected_qc_check_1_calls = [mock.call.process(vol)]
    expected_qc_check_2_calls = [mock.call.process(vol)]
    
    self.assertTrue(expected_qc_check_1_calls == self.qc_check_1_mock.mock_calls)
    self.assertTrue(expected_qc_check_2_calls == self.qc_check_2_mock.mock_calls)
    self.assertTrue(vol == result)

  def test_perform_quality_control_first_process_return_tuple(self):
    vol = object()
    a1 = object()
    
    self.qc_check_1_mock.process.return_value = (vol,a1)
    self.qc_check_2_mock.process.return_value = vol

    result = rave_pgf_apply_qc_plugin.perform_quality_control(vol, ["qc.check.1","qc.check.2"])
    
    expected_qc_check_1_calls = [mock.call.process(vol)]
    expected_qc_check_2_calls = [mock.call.process(vol)]
    
    self.assertTrue(expected_qc_check_1_calls == self.qc_check_1_mock.mock_calls)
    self.assertTrue(expected_qc_check_2_calls == self.qc_check_2_mock.mock_calls)
    self.assertTrue(vol == result)

  def test_generate_new_volume_with_qc(self):
    rave_pgf_quality_registry.add_plugin("poo", rave_overshooting_quality_plugin.rave_overshooting_quality_plugin())
    
    expected_no_of_scans = 10
    filename = "fixtures/pvol_seosu_20090501T120000Z.h5"
    
    DATE = "20160415"
    TIME = "100000"
    QC_LIST = "poo"
    
    args={}
    args["date"] = DATE
    args["time"] = TIME
    args["anomaly-qc"] = QC_LIST
     
    result = rave_pgf_apply_qc_plugin.generate_new_volume_with_qc(filename, args)
    
    self.assertEquals(expected_no_of_scans, result.getNumberOfScans())
    self.assertEquals(DATE, result.date)
    self.assertEquals(TIME, result.time)
    
    # Overshooting quality plugin will only add quality field to the first scan
    self.assertTrue(result.getScan(0).findQualityFieldByHowTask("se.smhi.detector.poo") != None, "Quality field not found")
