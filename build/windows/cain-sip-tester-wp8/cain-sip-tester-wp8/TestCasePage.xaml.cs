﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;

namespace cain_sip_tester_wp8
{
    public partial class TestCasePage : PhoneApplicationPage
    {
        public TestCasePage()
        {
            InitializeComponent();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            suiteName = NavigationContext.QueryString["SuiteName"];
            verbose = Convert.ToBoolean(NavigationContext.QueryString["Verbose"]);
            var tester = (Application.Current as App).tester;
            List<UnitTestCaseName> source = new List<UnitTestCaseName>();
            source.Add(new UnitTestCaseName("ALL"));
            for (int i = 0; i < tester.nbTests(suiteName); i++)
            {
                source.Add(new UnitTestCaseName(tester.testName(suiteName, i)));
            }

            Tests.ItemsSource = source;
            Tests.SelectionChanged += tests_selectionChanged;
        }

        void tests_selectionChanged(object sender, EventArgs e)
        {
            UnitTestCaseName test = (sender as LongListSelector).SelectedItem as UnitTestCaseName;
            NavigationService.Navigate(new Uri("/TestResultPage.xaml?SuiteName=" + suiteName + "&CaseName=" + test.Name + "&Verbose=" + verbose, UriKind.Relative));
        }

        private string suiteName;
        private bool verbose;
    }

    public class UnitTestCaseName
    {
        public string Name
        {
            get;
            set;
        }

        public UnitTestCaseName(string name)
        {
            this.Name = name;
        }
    }
}