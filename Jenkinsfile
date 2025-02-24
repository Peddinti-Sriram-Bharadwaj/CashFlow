pipeline {
    agent any

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Build') {
            steps {
                sh 'make'  // Run Makefile to compile the C project
            }
        }


        stage('Cleanup') {
            steps {
                sh 'make clean'  // Clean up compiled files
            }
        }
    }
}

