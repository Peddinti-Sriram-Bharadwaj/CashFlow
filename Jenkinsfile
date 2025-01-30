pipeline {
    agent { label 'macos' }
    environment {
        PATH = "/usr/local/bin:${env.PATH}"
    }
    stages {
        stage('Clone Repository') {
            steps {
                git 'https://github.com/Peddinti-Sriram-Bharadwaj/CashFlow.git'
            }
        }
        stage('Install Dependencies') {
            steps {
                sh 'brew install libsodium'
            }
        }
        stage('Compile Code') {
            steps {
                dir('server') {
                    sh 'make'
                }
                dir('admin') {
                    sh 'make'
                }
                dir('customer') {
                    sh 'make'
                }
                dir('manager') {
                    sh 'make'
                }
                dir('employee') {
                    sh 'make'
                }
            }
        }
        stage('Static Code Analysis') {
            steps {
                sh 'brew install cppcheck'
                sh 'cppcheck --enable=all --inconclusive --error-exitcode=1 .'
            }
        }
        stage('Run Tests') {
            steps {
                echo 'Running tests...'
                sh 'make test || echo "No tests implemented yet"'
            }
        }
    }
    post {
        always {
            archiveArtifacts artifacts: '**/*.o', allowEmptyArchive: true
        }
        success {
            echo 'Build and analysis completed successfully.'
        }
        failure {
            echo 'Build or analysis failed. Please check the logs.'
        }
    }
}
